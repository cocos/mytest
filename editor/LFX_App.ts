'use strict';

import { GFXAttributeName, LightComponent, ModelComponent, Node, Scene, Vec3 } from 'cc';
import exec from 'child_process';
import ps from 'path';
import { LFX_Baker, LFX_STAGE_DIRECT_LIGHTING, LFX_STAGE_END, LFX_STAGE_INDIRECT_LIGHTING, LFX_STAGE_POST_PROCESS, LFX_STAGE_START } from './LFX_Baker';
import { LFX_Light, LFX_Material, LFX_Mesh, LFX_Triangle, LFX_Vertex, LFX_World } from './LFX_Types';

// tslint:disable-next-line: class-name
export class LFX_App {
    public Baker: LFX_Baker = new LFX_Baker();
    private Models: ModelComponent[] = [];
    private Lights: LightComponent[] = [];
    // private Terrains: Terrain[] = [];

    private CurrentTicks: number = 0;
    private LastTicks: number = 0;

    public Init () {
        this.Baker.World = new LFX_World();
        this.Models = [];
        this.Lights = [];
        this.CurrentTicks = 0;
        this.LastTicks = 0;

        // 场景名字
        this.Baker.World.Name = 'Test';
        // 服务器地址
        // this.Baker.World.Settings.Server = 'localhost:9002';
        // 多重采样: 值(1, 2, 4, 8)
        this.Baker.World.Settings.MSAA = 4;
        // 烘培贴图大小: 值(128, 256, 512, 1024, 2048)
        this.Baker.World.Settings.Size = 1024;
        // Gamma值
        this.Baker.World.Settings.Gamma = 2.2;
        // 天空辐照度(用于全局光照)
        this.Baker.World.Settings.SkyRadiance = [0.5, 0.5, 0.5];
        // 全局光照缩放
        this.Baker.World.Settings.GIScale = 0; // 0: 关闭
        // 全局光照采样数
        this.Baker.World.Settings.GISamples = 10;
        // 全局光照光线最大跟踪次数
        this.Baker.World.Settings.GIPathLength = 4;
        // 线程数量
        this.Baker.World.Settings.Threads = 1;
    }

    public Run () {
        // 启动
        this.Baker.Launch(3000);

        // 启动烘焙程序
        const lfx_debug = false; // 调试模式从c++启动
        if (!lfx_debug) {
            const LFX_SERVER = this.Baker.lfxpath + '/LightFX';
            const process = exec.execFile(LFX_SERVER, { cwd: this.Baker.lfxpath });
        }

        // 定时器模拟更新
        const timer = setInterval(() => {
            if (this.Baker.closed) {
                clearInterval(timer);
                return;
            }

            if (!this.Baker.Started) {
                // 开始烘培
                if (this.Baker.connected) {
                    this.Baker.Start();

                    this.Baker.client.on('Tick', (data: any)=>{
                        this.LastTicks = this.CurrentTicks;
                    });
                }
            } else {
                // 结束了
                if (this.Baker.Finished) {
                    console.log('End');

                    const file = this.Baker.Download();

                    // 模型的Lightmap信息
                    for (let i = 0; i < file.MeshInfos.length; ++i) {
                        // 对应output/LFX_Mesh_xxxx.png

                        let info = file.MeshInfos[i];
                        console.log('Mesh ' + info.Id + ':' +
                        ' Index(' + info.Index + ') ' +
                        ' Offset(' + info.Offset[0] + ', ' + info.Offset[1] + ') ' +
                        ' Scale(' + info.Scale[0] + ', ' + info.Scale[1] + ')');
                    }

                    // 地形的Lightmap信息
                    for (const info of file.TerrainInfos) {
                        // 对应output/LFX_Terrain_xxxx.png
                    }

                    this.Baker.Stop();

                    clearInterval(timer);

                    this.Baker.Close();
                }
            }

            this.CurrentTicks += 100;
        }, 100);
    }

    public Export (node: Node) {
        if (!(node instanceof Scene)) {
            this._exportNode(node);
        }

        const HideInHierarchy = 1 << 10;

        for (const child of node.children) {
            if (!child.active) {
                continue;
            }
            if (child._objFlags & HideInHierarchy) {
                continue;
            }

            //this.Export(child);
        }

        // test
        if (1) {
            // ## 创建平面1 (接受阴影)
            //  v0-------v1
            //  |         |
            //  |         |
            //  |         |
            //  v2-------v3
            //
            const size = 128;

            const plane = new LFX_Mesh();
            plane.CastShadow = false;
            plane.RecieveShadow = true;
            plane.LightMapSize = 1024;

            // 顶点
            const v0 = new LFX_Vertex();
            v0.Position = [0, 0, 0];
            v0.Normal = [0, 1, 0];
            v0.UV = [0, 0];
            v0.LUV = [0, 0]
            plane.VertexBuffer.push(v0);

            const v1 = new LFX_Vertex();
            v1.Position = [size, 0, 0];
            v1.Normal = [0, 1, 0];
            v1.UV = [1, 0];
            v1.LUV = [1, 0];
            plane.VertexBuffer.push(v1);

            const v2 = new LFX_Vertex();
            v2.Position = [0, 0, size];
            v2.Normal = [0, 1, 0];
            v2.UV = [0, 1];
            v2.LUV = [0, 1];
            plane.VertexBuffer.push(v2);

            const v3 = new LFX_Vertex();
            v3.Position = [size, 0, size];
            v3.Normal = [0, 1, 0];
            v3.UV = [1, 1];
            v3.LUV = [1, 1];
            plane.VertexBuffer.push(v3);

            // 三角形
            const tri0 = new LFX_Triangle();
            tri0.Index = [0, 2, 1];
            tri0.MaterialId = 0;
            plane.TriangleBuffer.push(tri0);

            const tri1 = new LFX_Triangle();
            tri1.Index = [1, 2, 3];
            tri1.MaterialId = 0;
            plane.TriangleBuffer.push(tri1);

            // 材质
            const mtl = new LFX_Material();
            mtl.Diffuse = [1, 1, 1];
            mtl.Texture = this.Baker.World.AddUniqueTexture(''); // 相对路径
            plane.MaterialBuffer.push(mtl);

            this.Baker.World.Meshes.push(plane);

            // ##创建灯光
            //
            //
            const light = new LFX_Light();
            light.Type = LFX_Light.POINT;
            light.Position = [size / 2, size / 2, size / 2];
            light.Color = [0, 1, 0];
            light.CastShadow = true;
            light.GIEnable = false;
            light.AttenStart = 0;
            light.AttenEnd = size;

            this.Baker.World.Lights.push(light);
        }

        this.Baker.Upload('asset目录');
    }

    private _exportNode (node: Node) {
        /*
        const terrain = node.getComponent(Terrain);
        if (terrain != null && terrain.enabled) {
            this._exportTerrain(terrain);
        }
        */

        const models = node.getComponents(ModelComponent);
        for (const model of models) {
            if (model.enabled) {
                this._exportModel(model);
            }
        }

        const lights = node.getComponents(LightComponent);
        for (const light of lights) {
            if (light.enabled) {
                this._exportLight(light);
            }
        }
    }

    /*
    private _exportTerrain (terrain: Terrain) {
        const fxterrain = new LFX_Terrain();
        fxterrain.TileSize = terrain.info.tileSize;
        fxterrain.BlockCount[0] = terrain.info.blockCount[0];
        fxterrain.BlockCount[1] = terrain.info.blockCount[1];
        fxterrain.LightMapSize = terrain.info.lightMapSize;
        // @ts-ignore
        fxterrain.HeightField = terrain.getHeightField();

        this.Baker.World.Terrains.push(fxterrain);
        this.Terrains.push(terrain);
    }
    */

    private _exportModel (model: ModelComponent) {
        const mesh = model.mesh;
        if (mesh == null) {
            return false;
        }

        const fxmesh = new LFX_Mesh();
        fxmesh.CastShadow = true;
        fxmesh.RecieveShadow = false;
        fxmesh.LightMapSize = 0;

        const worldTM = model.node.getWorldMatrix();
        for (let iPrimitive = 0; iPrimitive < mesh.struct.primitives.length; ++iPrimitive) {
            const positions = mesh.readAttribute(iPrimitive, GFXAttributeName.ATTR_POSITION) as Float32Array;
            const normals = mesh.readAttribute(iPrimitive, GFXAttributeName.ATTR_NORMAL) as Float32Array;
            const uvs = mesh.readAttribute(iPrimitive, GFXAttributeName.ATTR_TEX_COORD) as Float32Array;
            const luvs = mesh.readAttribute(iPrimitive, GFXAttributeName.ATTR_TEX_COORD1) as Float32Array;
            const indices = mesh.readIndices(iPrimitive);

            // 检查是否有效
            if (!positions || !normals || !indices) {
                return false;
            }
            if (positions.length !== normals.length) {
                return false;
            }
            if (uvs != null && positions.length / 3 !== uvs.length / 2) {
                return false;
            }
            if (luvs != null && positions.length / 3 !== luvs.length / 2) {
                return false;
            }

            // 顶点数据
            for (let v = 0; v < positions.length / 3; ++v) {
                const P = new Vec3();
                P.x = positions[v * 3 + 0];
                P.y = positions[v * 3 + 1];
                P.z = positions[v * 3 + 2];
                Vec3.transformMat4(P, P, worldTM);

                const N = new Vec3();
                N.x = normals[v * 3 + 0];
                N.y = normals[v * 3 + 1];
                N.z = normals[v * 3 + 2];
                Vec3.transformMat4Normal(N, N, worldTM);

                const vertex = new LFX_Vertex();
                vertex.Position[0] = P.x;
                vertex.Position[1] = P.y;
                vertex.Position[2] = P.z;
                vertex.Normal[0] = N.x;
                vertex.Normal[1] = N.y;
                vertex.Normal[2] = N.z;
                if (uvs != null) {
                    vertex.UV[0] = uvs[v * 2 + 0];
                    vertex.UV[1] = uvs[v * 2 + 1];
                }
                if (luvs != null) {
                    vertex.LUV[0] = luvs[v * 2 + 0];
                    vertex.LUV[1] = luvs[v * 2 + 1];
                }
                fxmesh.VertexBuffer.push(vertex);
            }

            // 索引数据
            for (let i = 0; i < indices.length / 3; ++i) {
                const tri = new LFX_Triangle();
                if (indices.length < 256) {
                    tri.Index[0] = (indices as Uint8Array)[i * 3 + 0];
                    tri.Index[1] = (indices as Uint8Array)[i * 3 + 1];
                    tri.Index[2] = (indices as Uint8Array)[i * 3 + 2];
                }
                else if (indices.length < 65536) {
                    tri.Index[0] = (indices as Uint16Array)[i * 3 + 0];
                    tri.Index[1] = (indices as Uint16Array)[i * 3 + 1];
                    tri.Index[2] = (indices as Uint16Array)[i * 3 + 2];
                }
                else {
                    tri.Index[0] = (indices as Uint32Array)[i * 3 + 0];
                    tri.Index[1] = (indices as Uint32Array)[i * 3 + 1];
                    tri.Index[2] = (indices as Uint32Array)[i * 3 + 2];
                }

                tri.MaterialId = 0;

                fxmesh.TriangleBuffer.push(tri);
            }
        }

        // Materials
        const mtl = new LFX_Material();
        mtl.Diffuse[0] = 1;
        mtl.Diffuse[1] = 1;
        mtl.Diffuse[2] = 1;
        fxmesh.MaterialBuffer.push(mtl);

        this.Baker.World.Meshes.push(fxmesh);
        this.Models.push(model);
    }

    private _exportLight (light: LightComponent) {
        const fxlight = new LFX_Light();
        fxlight.Type = LFX_Light.DIRECTION;
        fxlight.Position[0] = light.node.getWorldPosition().x;
        fxlight.Position[1] = light.node.getWorldPosition().y;
        fxlight.Position[2] = light.node.getWorldPosition().z;

        const dir = new Vec3(0, 0, -1);
        Vec3.transformQuat(dir, dir, light.node.getWorldRotation());
        fxlight.Direction[0] = dir.x;
        fxlight.Direction[1] = dir.y;
        fxlight.Direction[2] = dir.z;

        fxlight.Color[0] = light.color.r;
        fxlight.Color[1] = light.color.g;
        fxlight.Color[2] = light.color.b;

        /*
        if (light instanceof SphereLightComponent) {
            const sl = light as SphereLightComponent;

            fxlight.Type = LFX_Light.POINT;
            fxlight.AttenStart = sl.size;
            fxlight.AttenEnd = sl.range;
            fxlight.AttenFallOff = 1;
        }
        else if (light instanceof SpotLightComponent) {
            const pl = light as SpotLightComponent;

            fxlight.Type = LFX_Light.SPOT;
            fxlight.SpotInner = 0;
            fxlight.SpotOuter = Math.cos(pl.spotAngle * (Math.PI / 180.0));
            fxlight.SpotFallOff = 1;
        }
        */

        fxlight.DirectScale = 1;
        fxlight.IndirectScale = 1;

        fxlight.GIEnable = false;
        fxlight.CastShadow = true;

        this.Baker.World.Lights.push(fxlight);
        this.Lights.push(light);
    }
}

export default LFX_App;