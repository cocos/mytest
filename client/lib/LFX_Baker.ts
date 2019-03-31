'use strict';

import ps from 'path';
import fs from 'fs';
import WebSocket from 'ws';
import { LFX_World, LFX_Buffer, LFX_File, LFX_TerrainLightMapInfo,
    LFX_MeshLightMapInfo } from './LFX_Types';

const LFX_FILE_VERSION = 0x2000;
const LFX_FILE_TERRAIN = 0x01;
const LFX_FILE_MESH = 0x02;
const LFX_FILE_LIGHT = 0x03;
const LFX_FILE_EOF = 0x00;

const LFX_PK_START = 1;
const LFX_PK_STOP = 2;
const LFX_PK_LOG = 777;
const LFX_PK_PROGRESS = 100;

//
export const LFX_STAGE_START = 0;
export const LFX_STAGE_DIRECT_LIGHTING = 1;
export const LFX_STAGE_INDIRECT_LIGHTING = 2;
export const LFX_STAGE_POST_PROCESS = 3;
export const LFX_STAGE_END = 4;

//
export class LFX_Baker {
    static Instance: LFX_Baker;

    World: LFX_World = new LFX_World;
    Started: boolean = false;
    Stage: number = 0;
    Progress: number = 0;

    //
    _connected: boolean = false;
    _socket: WebSocket | null = null;

    constructor() {
        LFX_Baker.Instance = this;
    }

    // 连接到服务器
    //
    Connect() {
        this._socket = new WebSocket('ws://localhost:9002');

        //
        this._socket.onopen = function(evt) {
            console.log('LFX Connection opened');
            LFX_Baker.Instance._connected = true;
        }

        //
        this._socket.onclose = function(evt) {
            console.log('LFX Connection closed');
            LFX_Baker.Instance._socket = null;
            LFX_Baker.Instance._connected = false;

            LFX_Baker.Instance.Started = false;
            LFX_Baker.Instance.Stage = 0;
            LFX_Baker.Instance.Progress = 0;
        }

        //
        this._socket.onmessage = function(evt) {
            let data = evt.data as Uint8Array;
            let buff = new LFX_Buffer;

            buff.Assign(data);

            LFX_Baker.Instance.ProcessMessage(buff);
        }

        //
        this._socket.onerror = function(evt) {
           console.log('LFX Connection error ' + evt.message);
        }
    }

    // 关闭
    //
    Close(): void {
        if (this._socket != null) {
            this._socket.close(0);
        }
        this._socket = null;
        this._connected = false;
    }

    // 发送消息
    //
    Send(pid: number, data: ArrayBuffer | null): void {
        let length = 4; // head
        if (data != null) {
            length += data.byteLength;
        }

        let buff = new ArrayBuffer(length);
        let dview = new DataView(buff);

        // message
        dview.setInt32(0, pid, true);
        if (data != null) {
            let dsrc = new DataView(data as ArrayBuffer);
            for (let i = 0; i < data.byteLength; ++i) {
                dview.setInt8(4 + i, dsrc.getInt8(0));
            }
        }

        if (this._socket != null) {
            this._socket.send(buff);
        }
    }

    // 上传
    //
    delDir(path: string) {
        if(fs.existsSync(path)){

        }
    }

    Upload(asset_path: string) {
        let buff = new LFX_Buffer;

        let immediatePath = ps.resolve('lightfx');
        if (fs.existsSync(immediatePath)) {
            // 删除零时文件
            let files = fs.readdirSync(immediatePath);
            files.forEach((file, index) => {
                let curPath = immediatePath + '/' + file;
                if(!fs.statSync(curPath).isDirectory()){
                    fs.unlinkSync(curPath);
                }
            });
        }
        else {
            fs.mkdirSync(immediatePath);
        }

        let outputPath = ps.resolve('lightfx/output');
        if (fs.existsSync(outputPath)) {
            // 删除输出文件
            let files = fs.readdirSync(outputPath);
            files.forEach((file, index) => {
                let curPath = outputPath + '/' + file;
                if(!fs.statSync(curPath).isDirectory()){
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
        buff.WriteInt32(this.World.Settings.Threads);

        // Meshes
        for (let i = 0; i < this.World.Meshes.length; ++i) {
            buff.WriteInt32(LFX_FILE_MESH);

            let mesh = this.World.Meshes[i];

            buff.WriteString(mesh.Id);
            buff.WriteInt8(mesh.CastShadow ? 1 : 0);
            buff.WriteInt8(mesh.RecieveShadow ? 1 : 0);
            buff.WriteInt32(mesh.LightMapSize);
            buff.WriteInt32(mesh.VertexBuffer.length);
            buff.WriteInt32(mesh.TriangleBuffer.length);
            buff.WriteInt32(mesh.MaterialBuffer.length);

            for (let v = 0; v < mesh.VertexBuffer.length; ++v) {
                let vtx = mesh.VertexBuffer[v];

                buff.WriteFloatArray(vtx.Position);
                buff.WriteFloatArray(vtx.Normal);
                buff.WriteFloatArray(vtx.UV);
                buff.WriteFloatArray(vtx.LUV);
            }

            for (let t = 0; t < mesh.TriangleBuffer.length; ++t) {
                let tri = mesh.TriangleBuffer[t];

                buff.WriteIntArray(tri.Index);
                buff.WriteInt32(tri.MaterialId);
            }

            for (let m = 0; m < mesh.MaterialBuffer.length; ++m) {
                let mtl = mesh.MaterialBuffer[m];

                buff.WriteFloatArray(mtl.Diffuse);
                buff.WriteString(mtl.Texture);
            }
        }

        // Lights
        for (let i = 0; i < this.World.Lights.length; ++i) {
            buff.WriteInt32(LFX_FILE_LIGHT);

            let light = this.World.Lights[i];

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
        //
        for (let i = 0; i < this.World.Textures.length; ++i) {
            let data = fs.readFileSync(asset_path + '/' + this.World.Textures[i]);
            let target = this.World.Textures[i].replace('/', '$');

            fs.writeFileSync(immediatePath + '/' + target, data);
        }
    }

    //
    Download() {
        let file = new LFX_File;

        let outputPath = ps.resolve('lightfx/output');

        let buff = fs.readFileSync(ps.resolve(outputPath, 'lfx.o'));
        if (buff != null) {
            let stream = new LFX_Buffer;
            stream.Assign(buff);

            file.Verison = stream.ReadInt();

            do {
                let cid = stream.ReadInt();
                if (cid == LFX_FILE_EOF) {
                    break;
                }

                if (cid == LFX_FILE_TERRAIN) {
                    let count = stream.ReadInt();
                    for (let i = 0; i < count; ++i) {
                        let info = new LFX_TerrainLightMapInfo;

                        info.Index = stream.ReadInt();
                        info.Offset[0] = stream.ReadFloat();
                        info.Offset[1] = stream.ReadFloat();
                        info.Scale[0] = stream.ReadFloat();
                        info.Scale[1] = stream.ReadFloat();

                        file.TerrainInfos.push(info);
                    }
                }
                else if (cid == LFX_FILE_MESH) {
                    let count = stream.ReadInt();
                    for (let i = 0; i < count; ++i) {
                        let info = new LFX_MeshLightMapInfo;

                        info.Id = stream.ReadInt();
                        info.Index = stream.ReadInt();
                        info.Offset[0] = stream.ReadFloat();
                        info.Offset[1] = stream.ReadFloat();
                        info.Scale[0] = stream.ReadFloat();
                        info.Scale[1] = stream.ReadFloat();

                        file.MeshInfos.push(info);
                    }
                }
                else {
                    // error
                    console.log('LightFX unknown chunk ' + cid);
                }

            } while (1);
        }

        return file;
    }

    // 开始烘培
    //
    Start() {
        this.Send(LFX_PK_START, null);
        this.Started = true;
    }

    // 停止烘培
    //
    Stop() {
        this.Send(LFX_PK_STOP, null);
    }

    // 更新消息
    //
    ProcessMessage(data: LFX_Buffer) {
        let pid = data.ReadInt();

        if (pid == LFX_PK_PROGRESS) {
            this.OnProgress(data);
        }
        else if (pid == LFX_PK_LOG) {
            this.OnLog(data);
        }
        else {
            console.log('Unknown message ' + pid);
        }
    }

    // 进度消息处理函数
    //
    OnProgress(data: LFX_Buffer) {
        // State: int
        // Progress: int
        //
        let Stage = data.ReadInt();
        let Progress = data.ReadInt();

        // do some thing
        //
        this.Stage = Stage;
        this.Progress = Progress;
    }

    // 日志消息处理函数
    //
    OnLog(data: LFX_Buffer) {
       // Channel: int
       // Length: int
       // Message: string
       //
       let Channel = data.ReadInt();
       let Length = data.ReadInt();
       let Message = data.ReadString();

        // do some thing
        //
        console.log(Message);
    }
}
