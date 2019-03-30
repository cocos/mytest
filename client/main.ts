import { LFX_Vertex, LFX_Mesh, LFX_Triangle, LFX_Material, LFX_Light, LFX_File } from './lib/LFX_Types';
import { LFX_Baker, LFX_STAGE_START, LFX_STAGE_DIRECT_LIGHTING, LFX_STAGE_INDIRECT_LIGHTING, LFX_STAGE_POST_PROCESS, LFX_STAGE_END } from './lib/LFX_Baker';
import exec = require('child_process')

let Baker = new LFX_Baker;

// 1.创建场景
//

// ## 烘培设置
//
if (1) {
    // 场景名字
    Baker.World.Name = "Test";
    // 服务器地址
    Baker.World.Settings.Server = "localhost:9002";
    // 环境光照
    Baker.World.Settings.Ambient = [0.0, 0.0, 0.0];
    // 天空辐照度(用于全局光照)
    Baker.World.Settings.SkyRadiance = [0.5, 0.5, 0.5];
    // 多重采样: 值(1, 2, 4, 8)
    Baker.World.Settings.MSAA = 4;
    // 烘培贴图大小: 值(128, 256, 512, 1024, 2048)
    Baker.World.Settings.Size = 1024;
    // Gamma值
    Baker.World.Settings.Gamma = 2.2;
    // 全局光照缩放
    Baker.World.Settings.GIScale = 0; // 0: 关闭
    // 全局光照采样数
    Baker.World.Settings.GISamples = 10;
    // 全局光照光线最大跟踪次数
    Baker.World.Settings.GIPathLength = 4;
    // 线程数量
    Baker.World.Settings.Threads = 1;
}

// ## 创建平面1 (接受阴影)
//  v0-------v1
//  |         |
//  |         |
//  |         |
//  v2-------v3
//
if (1) {
    let plane = new LFX_Mesh;
    plane.Id = "plane1";
    plane.CastShadow = false;
    plane.RecieveShadow = true;
    plane.LightMapSize = 1024;

    // 顶点
    let v0 = new LFX_Vertex;
    v0.Position = [0, 0, 0];
    v0.Normal = [0, 1, 0];
    v0.UV = [0, 0];
    v0.LUV = [0, 0]
    plane.VertexBuffer.push(v0);
    
    let v1 = new LFX_Vertex;
    v1.Position = [100, 0, 0];
    v1.Normal = [0, 1, 0];
    v1.UV = [1, 0];
    v1.LUV = [1, 0];
    plane.VertexBuffer.push(v1);
    
    let v2 = new LFX_Vertex;
    v2.Position = [0, 0, 100];
    v2.Normal = [0, 1, 0];
    v2.UV = [0, 1];
    v2.LUV = [0, 1];
    plane.VertexBuffer.push(v2);
    
    let v3 = new LFX_Vertex;
    v3.Position = [100, 0, 100];
    v3.Normal = [0, 1, 0];
    v3.UV = [1, 1];
    v3.LUV = [1, 1];
    plane.VertexBuffer.push(v3);
    
    // 三角形
    let tri0 = new LFX_Triangle;
    tri0.Index = [0, 2, 1];
    tri0.MaterialId = 0;
    plane.TriangleBuffer.push(tri0);

    let tri1 = new LFX_Triangle;
    tri1.Index = [1, 2, 3];
    tri1.MaterialId = 0;
    plane.TriangleBuffer.push(tri1);

    // 材质
    let mtl = new LFX_Material;
    mtl.Diffuse = [1, 1, 1];
    mtl.Texture = Baker.World.AddUniqueTexture(""); // 相对路径
    plane.MaterialBuffer.push(mtl);

    Baker.World.Meshes.push(plane);
}

// ## 创建平面2 (投射阴影)
//
//
if (1) {
    let plane = new LFX_Mesh;
    plane.Id = "plane2";
    plane.CastShadow = true;
    plane.RecieveShadow = false;
    plane.LightMapSize = 0;

   // 顶点
    let v0 = new LFX_Vertex;
    v0.Position = [0, 10, 0];
    v0.Normal = [0, 1, 0];
    v0.UV = [0, 0];
    v0.LUV = [0, 0]
    plane.VertexBuffer.push(v0);
    
    let v1 = new LFX_Vertex;
    v1.Position = [50, 10, 0];
    v1.Normal = [0, 1, 0];
    v1.UV = [1, 0];
    v1.LUV = [1, 0];
    plane.VertexBuffer.push(v1);
    
    let v2 = new LFX_Vertex;
    v2.Position = [0, 10, 50];
    v2.Normal = [0, 1, 0];
    v2.UV = [0, 1];
    v2.LUV = [0, 1];
    plane.VertexBuffer.push(v2);
    
    let v3 = new LFX_Vertex;
    v3.Position = [50, 10, 50];
    v3.Normal = [0, 1, 0];
    v3.UV = [1, 1];
    v3.LUV = [1, 1];
    plane.VertexBuffer.push(v3);

    for (let i = 0; i < 4; ++i) {
        plane.VertexBuffer[i].Position[0] += 25;
        plane.VertexBuffer[i].Position[2] += 25;
    }
    
    // 三角形
    let tri0 = new LFX_Triangle;
    tri0.Index = [0, 2, 1];
    tri0.MaterialId = 0;
    plane.TriangleBuffer.push(tri0);

    let tri1 = new LFX_Triangle;
    tri1.Index = [1, 2, 3];
    tri1.MaterialId = 0;
    plane.TriangleBuffer.push(tri1);

    // 材质
    let mtl = new LFX_Material;
    mtl.Diffuse = [1, 1, 1];
    mtl.Texture = Baker.World.AddUniqueTexture(""); // 相对路径
    plane.MaterialBuffer.push(mtl);

    Baker.World.Meshes.push(plane);
}

// ##创建灯光1
//
//
if (1) {
    let light = new LFX_Light;
    light.Type = LFX_Light.POINT;
    light.Position = [0, 100, 0];
    light.Color = [1, 0, 0];
    light.CastShadow = true;
    light.GIEnable = false;
    light.AttenStart = 0;
    light.AttenEnd = 125;

    Baker.World.Lights.push(light);
}

// ##创建灯光2
//
//
if (1) {
    let light = new LFX_Light;
    light.Type = LFX_Light.POINT;
    light.Position = [100, 100, 0];
    light.Color = [0, 1, 0];
    light.CastShadow = true;
    light.GIEnable = false;
    light.AttenStart = 0;
    light.AttenEnd = 125;

    Baker.World.Lights.push(light);
}

// ##创建灯光3
//
//
if (1) {
    let light = new LFX_Light;
    light.Type = LFX_Light.POINT;
    light.Position = [0, 100, 100];
    light.Color = [0, 0, 1];
    light.CastShadow = true;
    light.GIEnable = false;
    light.AttenStart = 0;
    light.AttenEnd = 125;

    Baker.World.Lights.push(light);
}

// ##创建灯光4
//
//
if (1) {
    let light = new LFX_Light;
    light.Type = LFX_Light.POINT;
    light.Position = [100, 100, 100];
    light.Color = [1, 1, 1];
    light.CastShadow = true;
    light.GIEnable = false;
    light.AttenStart = 0;
    light.AttenEnd = 125;

    Baker.World.Lights.push(light);
}

// 2. 应用框架
//
class App {
    _baker: LFX_Baker = Baker;
    _stage: number = -1;
    _progress: number = -1;

    constructor() {
        // 启动服务器
        exec.execFile(process.cwd() + "/LightFX.exe");

        // 链接服务器
        this._baker.Connect();

        // 定时器模拟更新
        let timer = setInterval(() => {
            if (this._baker._socket == null) {
                return;
            }

            if (!this._baker.Started) {
                // 开始烘培
                if (this._baker._connected) {
                    this._baker.Upload("asset目录");
                    this._baker.Start();
                }
            }
            else {
                // 更新状态
                if (this._stage != this._baker.Stage) {
                    this._stage = this._baker.Stage;
                    if (this._stage == LFX_STAGE_START) {
                        console.log("Start");
                    }
                    else if (this._stage == LFX_STAGE_DIRECT_LIGHTING) {
                        console.log("Direct lighting");
                    }
                    else if (this._stage == LFX_STAGE_INDIRECT_LIGHTING) {
                        console.log("Indirect lighting");
                    }
                    else if (this._stage == LFX_STAGE_POST_PROCESS) {
                        console.log("Post process");
                    }
                } 

                // 更新进度条
                if (this._progress != this._baker.Progress) {
                    
                    this._progress = this._baker.Progress
                    console.log(this._progress);
                }

                // 结束了
                if (this._stage == LFX_STAGE_END) {
                    console.log("End");

                    let file = this._baker.Download();

                    // 模型的Lightmap信息
                    for (let i = 0; i < file.MeshInfos.length; ++i) {
                        // 对应output/LFX_Mesh_xxxx.png

                        let info = file.MeshInfos[i];
                        console.log("Mesh " + info.Id + ":" +
                        " Index(" + info.Index + ") " +
                        " Offset(" + info.Offset[0] + ", " + info.Offset[1] + ") " +
                        " Scale(" + info.Scale[0] + ", " + info.Scale[1] + ")");
                    }

                    // 地形的Lightmap信息
                    for (let i = 0; i < file.TerrainInfos.length; ++i) {
                        // 对应output/LFX_Terrain_xxxx.png
                    }

                    this._baker.Stop();

                    clearInterval(timer);
                }
            }
            
        }, 100);
    }
}


export = new App;