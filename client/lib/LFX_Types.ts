'use strict';

// Settings 编辑器要提供编辑界面
//
export class LFX_Settings {
    Server: string = "localhost:9002";

    // 环境光照
    Ambient: number[] = [0, 0, 0];
    // 天空辐照度(用于全局光照)
    SkyRadiance: number[] = [0, 0, 0];
    // 多重采样: 值(1, 2, 4, 8)
    MSAA = 4;
    // 烘培贴图大小: 值(128, 256, 512, 1024, 2048)
    Size = 1024;
    // Gamma值
    Gamma = 2.2;
    // 全局光照缩放
    GIScale = 0.5;
    // 全局光照采样数
    GISamples = 25;
    // 全局光照光线最大跟踪次数
    GIPathLength = 4;
    // 线程数量
    Threads = 1;
}

export class LFX_Vertex {
    Position: number[] = [0, 0, 0];
    Normal: number[] = [0, 0, 0];
    UV: number[] = [0, 0];
    LUV: number[] = [0, 0];
}

export class LFX_Triangle {
    Index: number[] = [0, 0, 0];
    MaterialId: number = 0;
}

export class LFX_Material {
    //
    Diffuse: number[] = [1, 1, 1];
    // 纹理，(所有'/'或者'\\'都必须转换成'$')
    Texture: string = "";
}

// 模型
//
export class LFX_Mesh {
    Id: string = "";
    CastShadow: boolean = false;
    RecieveShadow: boolean = false;
    LightMapSize: number = 0;
    VertexBuffer: Array<LFX_Vertex> = new Array<LFX_Vertex>();
    TriangleBuffer: Array<LFX_Triangle> = new Array<LFX_Triangle>();
    MaterialBuffer: Array<LFX_Material> = new Array<LFX_Material>();
}

// 灯光
//
export class LFX_Light {
    static POINT = 0;
    static SPOT = 1;
    static DIRECTION = 2;

    // 类型
    Type: number = LFX_Light.POINT;
    // 位置
    Position: number[] = [0, 0, 0];
    // 方向
    Direction: number[] = [0, 0, 0];
    // 颜色
    Color: number[] = [0, 0, 0];

    // 衰减开始距离
    AttenStart: number = 0;
    // 衰减结束距离
    AttenEnd: number = 1;
    // 衰减强度
    AttenFallOff: number = 1;

    // 聚光灯内角
    SpotInner: number = 1;
    // 聚光灯外角
    SpotOuter: number = 0.7071;
    // 聚光灯衰减强度
    SpotFallOff: number = 1;

    // 直接光照缩放
    DirectScale: number = 1;
    // 间接光照缩放
    IndirectScale: number = 1;

    // 是否开启全局光照
    GIEnable: boolean = false;
    // 是否投射阴影
    CastShadow: boolean = false;
}

//
//
export class LFX_Buffer {
    Length: number = 0;
    Buffer: Uint8Array = new Uint8Array(2048);
    _dview: DataView = new DataView(this.Buffer.buffer);

    //
    _seekPos: number = 0;

    Reserve(size: number) {
        if (this.Buffer.byteLength > size) {
            return;
        }

        let capacity = this.Buffer.byteLength;
        while (capacity < size) {
            capacity += capacity;
        }

        let temp = new Uint8Array(capacity);
        for (let i = 0; i < this.Length; ++i) {
            temp[i] = this.Buffer[i];
        }

        this.Buffer = temp;
        this._dview = new DataView(this.Buffer.buffer);
    }

    Assign(buff: Uint8Array) {
        this.Buffer = buff;
        this.Length = buff.length;
        this._seekPos = buff.byteOffset;
        this._dview = new DataView(buff.buffer);
    }

    WriteInt8(value: number) {
        this.Reserve(this.Length + 1);

        this._dview.setInt8(this.Length, value);
        this.Length += 1;
    }

    WriteInt16(value: number) {
        this.Reserve(this.Length + 2);

        this._dview.setInt16(this.Length, value, true);
        this.Length += 2;
    }

    WriteInt32(value: number) {
        this.Reserve(this.Length + 4);

        this._dview.setInt32(this.Length, value, true);
        this.Length += 4;
    }

    WriteIntArray(value: number[]) {
        this.Reserve(this.Length + 4 * value.length);

        for (let i = 0; i < value.length; ++i) {
            this._dview.setInt32(this.Length + i * 4, value[i], true);
        }
        this.Length += 4 * value.length;
    }

    WriteFloat(value: number) {
        this.Reserve(this.Length + 4);

        this._dview.setFloat32(this.Length, value, true);
        this.Length += 4;
    }

    WriteFloatArray(value: number[]) {
        this.Reserve(this.Length + 4 * value.length);

        for (let i = 0; i < value.length; ++i) {
            this._dview.setFloat32(this.Length + i * 4, value[i], true);
        }
        this.Length += 4 * value.length;
    }

    WriteString(value: string) {
        this.Reserve(this.Length + value.length + 4);

        this._dview.setInt32(this.Length, value.length, true);
        for (let i = 0; i < value.length; ++i) {
            this._dview.setInt8(this.Length + 4 + i, value.charCodeAt(i));
        }
        this.Length += value.length + 4;
    }

    ReadInt8() {
        let value = this._dview.getInt8(this._seekPos);
        this._seekPos += 1;
        return value;
    }

    ReadInt16() {
        let value = this._dview.getInt16(this._seekPos, true);
        this._seekPos += 2;
        return value;
    }

    ReadInt() {
        let value = this._dview.getInt32(this._seekPos, true);
        this._seekPos += 4;
        return value;
    }

    ReadIntArray(value: number[]) {
        for (let i = 0; i < value.length; ++i) {
            value[i] = this._dview.getInt32(this._seekPos + i * 4, true);
        }
        this._seekPos += 4 * value.length;
        return value;
    }

    ReadFloat() {
        let value = this._dview.getFloat32(this._seekPos, true);
        this._seekPos += 4;
        return value;
    }

    ReadFloatArray(value: number[]) {
        for (let i = 0; i < value.length; ++i) {
            value[i] = this._dview.getFloat32(this._seekPos + i * 4, true);
        }
        this._seekPos += 4 * value.length;
        return value;
    }

    ReadString() {
        let length = this.ReadInt();

        let value = "";
        for (let i = 0; i < length; ++i) {
            value += String.fromCharCode(this.ReadInt8());
        }

        return value;
    }
}

// World
//
export class LFX_World {
    Name: string = "";
    Settings: LFX_Settings = new LFX_Settings;
    Textures: Array<string> = new Array<string>();
    Meshes: Array<LFX_Mesh> = new Array<LFX_Mesh>();
    Lights: Array<LFX_Light> = new Array<LFX_Light>();

    AddUniqueTexture(tex: string) {
        if (tex != "") {
            for (let i = 0; i < this.Textures.length; ++i) {
                if (this.Textures[i] == tex) {
                    return tex;
                }
            }

            this.Textures.push(tex);
        }

        return tex;
    }
}

// LightMapInfo
//
export class LFX_TerrainLightMapInfo {
    Index: number = 0;
    Offset: number[] = [0, 0];
    Scale: number[] = [0, 0];
}

export class LFX_MeshLightMapInfo {
    // 模型Id(创建时候的索引)
    Id: number = 0;
    //
    Index: number = 0;
    Offset: number[] = [0, 0];
    Scale: number[] = [0, 0];
}

export class LFX_File {
    Verison: number = 0;
    MeshInfos: Array<LFX_MeshLightMapInfo> = new Array<LFX_MeshLightMapInfo>();
    TerrainInfos: Array<LFX_TerrainLightMapInfo> = new Array<LFX_TerrainLightMapInfo>();
}
