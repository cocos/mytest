'use strict';

import fs from 'fs';
import ps from 'path';
const io = require('socket.io');
import { LFXBuffer, LFXFile, LFXMeshLightMapInfo, LFXTerrainLightMapInfo, LFXWorld } from './LFX_Types';

const LFX_FILE_VERSION = 0x2000;
const LFX_FILE_TERRAIN = 0x01;
const LFX_FILE_MESH = 0x02;
const LFX_FILE_LIGHT = 0x03;
const LFX_FILE_EOF = 0x00;

/*
export const LFX_STAGE_START = 0;
export const LFX_STAGE_DIRECT_LIGHTING = 1;
export const LFX_STAGE_INDIRECT_LIGHTING = 2;
export const LFX_STAGE_AMBIENT_OCCLUSION = 3;
export const LFX_STAGE_POST_PROCESS = 4;
export const LFX_STAGE_END = 5;
*/

export class LFXBaker {
    public static Instance: LFXBaker;

    public World: LFXWorld = new LFXWorld();
    public Started: boolean = false;
    public Finished: boolean = false;

    //
    private _server: any = null;
    private _client: any = null;

    constructor () {
        LFXBaker.Instance = this;
    }

    get lfxpath () {
        return ps.resolve('lightfx');
    }

    get closed () {
        return this._server == null;
    }

    get connected () {
        return this._client != null;
    }

    get client () {
        return this._client;
    }

    // 启动socket.io
    public Launch (port: number) {
        this.Started = false;
        this.Finished = false;

        this._server = io.listen(port);

        this._server.on('connection', (socket: any) => {
            socket.on('Login', (data: any) => {
                this._client = socket;
            });

            socket.on('Log', (data: any) => {
                console.log(data);
            });

            socket.on('Progress', (data: any) => {
                console.log(data);
            });

            socket.on('Finished', (data: any) => {
                this.Finished = true;
            });
        });
    }

    // 关闭
    public Close (): void {
        if (this._server != null) {
            this._server.disconnect();
        }
        this._server = null;
        this._client = null;
    }

    // 上传
    public Upload (asset_path: string) {
        const buff = new LFXBuffer();

        const immediatePath = this.lfxpath;
        if (fs.existsSync(immediatePath)) {
            // 删除零时文件
            const files = fs.readdirSync(immediatePath);
            files.forEach((file, index) => {
                const curPath = immediatePath + '/' + file;
                if(!fs.statSync(curPath).isDirectory()){
                    fs.unlinkSync(curPath);
                }
            });
        } else {
            fs.mkdirSync(immediatePath);
        }

        const outputPath = this.lfxpath + '/output';
        if (fs.existsSync(outputPath)) {
            // 删除输出文件
            const files = fs.readdirSync(outputPath);
            files.forEach((file, index) => {
                const curPath = outputPath + '/' + file;
                if (!fs.statSync(curPath).isDirectory()) {
                    fs.unlinkSync(curPath);
                }
            });
        }

        // Head
        buff.WriteInt32(LFX_FILE_VERSION);
        buff.WriteString(this.World.Name);

        // Setting
        buff.WriteFloatArray(this.World.Settings.Ambient);
        buff.WriteFloatArray(this.World.Settings.SkyRadiance);
        buff.WriteInt32(this.World.Settings.MSAA);
        buff.WriteInt32(this.World.Settings.Size);
        buff.WriteFloat(this.World.Settings.Gamma);
        buff.WriteFloat(this.World.Settings.GIScale);
        buff.WriteInt32(this.World.Settings.GISamples);
        buff.WriteInt32(this.World.Settings.GIPathLength);
        buff.WriteInt32(this.World.Settings.AOLevel);
        buff.WriteFloat(this.World.Settings.AOStrength);
        buff.WriteFloat(this.World.Settings.AORadius);
        buff.WriteFloatArray(this.World.Settings.AOColor);
        buff.WriteInt32(this.World.Settings.Threads);

        // Terrains
        for (const terrain of this.World.Terrains) {
            buff.WriteInt32(LFX_FILE_TERRAIN);
            buff.WriteFloat(terrain.TileSize);
            buff.WriteIntArray(terrain.BlockCount);
            buff.WriteInt32(terrain.LightMapSize);
            buff.WriteFloatArray(terrain.HeightField);
        }

        // Meshes
        for (const mesh of this.World.Meshes) {
            buff.WriteInt32(LFX_FILE_MESH);
            buff.WriteInt8(mesh.CastShadow ? 1 : 0);
            buff.WriteInt8(mesh.RecieveShadow ? 1 : 0);
            buff.WriteInt32(mesh.LightMapSize);
            buff.WriteInt32(mesh.VertexBuffer.length);
            buff.WriteInt32(mesh.TriangleBuffer.length);
            buff.WriteInt32(mesh.MaterialBuffer.length);

            for (const vtx of mesh.VertexBuffer) {
                buff.WriteFloatArray(vtx.Position);
                buff.WriteFloatArray(vtx.Normal);
                buff.WriteFloatArray(vtx.UV);
                buff.WriteFloatArray(vtx.LUV);
            }

            for (const tri of mesh.TriangleBuffer) {
                buff.WriteIntArray(tri.Index);
                buff.WriteInt32(tri.MaterialId);
            }

            for (const mtl of mesh.MaterialBuffer) {
                buff.WriteFloatArray(mtl.Diffuse);
                buff.WriteString(mtl.Texture);
            }
        }

        // Lights
        for (const light of this.World.Lights) {
            buff.WriteInt32(LFX_FILE_LIGHT);
            buff.WriteInt32(light.Type);
            buff.WriteFloatArray(light.Position);
            buff.WriteFloatArray(light.Direction);
            buff.WriteFloatArray(light.Color);
            buff.WriteFloat(light.AttenStart);
            buff.WriteFloat(light.AttenEnd);
            buff.WriteFloat(light.AttenFallOff);
            buff.WriteFloat(light.SpotInner);
            buff.WriteFloat(light.SpotOuter);
            buff.WriteFloat(light.SpotFallOff);
            buff.WriteFloat(light.DirectScale);
            buff.WriteFloat(light.IndirectScale);
            buff.WriteInt8(light.GIEnable ? 1 : 0);
            buff.WriteInt8(light.CastShadow ? 1 : 0);
        }

        // EOF
        buff.WriteInt32(LFX_FILE_EOF);

        // Save
        fs.writeFileSync(immediatePath + '/lfx.i', buff.Buffer);

        // Copy Textures
        for (const tex of this.World.Textures) {
            const data = fs.readFileSync(asset_path + '/' + tex);
            const target = tex.replace('/', '$');

            fs.writeFileSync(immediatePath + '/' + target, data);
        }
    }

    //
    public Download () {
        const file = new LFXFile();
        const filename = this.lfxpath + '/output/lfx.o';

        const buff = fs.readFileSync(filename);
        if (buff != null) {
            const stream = new LFXBuffer();
            stream.Assign(buff);

            file.Verison = stream.ReadInt();

            do {
                const cid = stream.ReadInt();
                if (cid === LFX_FILE_EOF) {
                    break;
                }

                if (cid === LFX_FILE_TERRAIN) {
                    const count = stream.ReadInt();
                    for (let i = 0; i < count; ++i) {
                        const info = new LFXTerrainLightMapInfo();
                        info.Id = stream.ReadInt();
                        info.Index = stream.ReadInt();
                        info.Offset[0] = stream.ReadFloat();
                        info.Offset[1] = stream.ReadFloat();
                        info.Scale[0] = stream.ReadFloat();
                        info.Scale[1] = stream.ReadFloat();

                        file.TerrainInfos.push(info);
                    }
                } else if (cid === LFX_FILE_MESH) {
                    const count = stream.ReadInt();
                    for (let i = 0; i < count; ++i) {
                        const info = new LFXMeshLightMapInfo();

                        info.Id = stream.ReadInt();
                        info.Index = stream.ReadInt();
                        info.Offset[0] = stream.ReadFloat();
                        info.Offset[1] = stream.ReadFloat();
                        info.Scale[0] = stream.ReadFloat();
                        info.Scale[1] = stream.ReadFloat();

                        file.MeshInfos.push(info);
                    }
                } else {
                    // error
                    console.log('LightFX unknown chunk ' + cid);
                }

            } while (1);
        }

        return file;
    }

    // 开始烘培
    public Start () {
        this._client!.emit('Start');
        this.Started = true;
    }

    // 停止烘培
    public Stop () {
        this._client!.emit('Stop');
    }
}
