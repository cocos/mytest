
// Settings 编辑器要提供编辑界面
//
export class LFXSettings {
    public Server: string = 'localhost:9002';

    // 环境光照
    public Ambient: number[] = [0, 0, 0];
    // 天空辐照度(用于全局光照)
    public SkyRadiance: number[] = [0, 0, 0];
    // 多重采样: 值(1, 2, 4, 8)
    public MSAA = 4;
    // 烘培贴图大小: 值(128, 256, 512, 1024, 2048)
    public Size = 1024;
    // Gamma值
    public Gamma = 2.2;
    // 全局光照缩放
    public GIScale = 0.5;
    // 全局光照采样数
    public GISamples = 25;
    // 全局光照光线最大跟踪次数
    public GIPathLength = 4;
    // AO等级(0：关闭，1：低，2：高)
    public AOLevel = 0;
    // AO强度
    public AOStrength = 0.5;
    // AO影响的范围
    public AORadius = 1.0;
    // AO颜色
    public AOColor: number[] = [0.5, 0.5, 0.5];
    // 线程数量
    public Threads = 1;
}

export class LFXVertex {
    public Position: number[] = [0, 0, 0];
    public Normal: number[] = [0, 0, 0];
    public  UV: number[] = [0, 0];
    public LUV: number[] = [0, 0];
}

export class LFXTriangle {
    public Index: number[] = [0, 0, 0];
    public MaterialId: number = 0;
}

export class LFXMaterial {
    //
    public Diffuse: number[] = [1, 1, 1];
    // 纹理，(所有'/'或者'\\'都必须转换成'$')
    public Texture: string = '';
}

// 模型
//
export class LFXMesh {
    public CastShadow: boolean = false;
    public RecieveShadow: boolean = false;
    public LightMapSize: number = 0;
    public VertexBuffer: LFXVertex[] = new Array<LFXVertex>();
    public TriangleBuffer: LFXTriangle[] = new Array<LFXTriangle>();
    public MaterialBuffer: LFXMaterial[] = new Array<LFXMaterial>();
}

// 地形
export class LFXTerrain {
    public TileSize: number = 0;
    public BlockCount: number[] = [0, 0];
    public HeightField: number[] = [];
    public  LightMapSize: number = 0;
}

// 灯光
//
export class LFXLight {
    public static POINT = 0;
    public static SPOT = 1;
    public static DIRECTION = 2;

    // 类型
    public Type: number = LFXLight.POINT;
    // 位置
    public Position: number[] = [0, 0, 0];
    // 方向
    public Direction: number[] = [0, 0, 0];
    // 颜色
    public Color: number[] = [0, 0, 0];

    // 衰减开始距离
    public AttenStart: number = 0;
    // 衰减结束距离
    public AttenEnd: number = 1;
    // 衰减强度
    public AttenFallOff: number = 1;

    // 聚光灯内角
    public SpotInner: number = 1;
    // 聚光灯外角
    public SpotOuter: number = 0.7071;
    // 聚光灯衰减强度
    public SpotFallOff: number = 1;

    // 直接光照缩放
    public DirectScale: number = 1;
    // 间接光照缩放
    public IndirectScale: number = 1;

    // 是否开启全局光照
    public GIEnable: boolean = false;
    // 是否投射阴影
    public CastShadow: boolean = false;
}

export class LFXBuffer {
    public Length: number = 0;
    public Buffer: Uint8Array = new Uint8Array(2048);
    private _dview: DataView = new DataView(this.Buffer.buffer);
    private _seekPos: number = 0;

    public Reserve (size: number) {
        if (this.Buffer.byteLength > size) {
            return;
        }

        let capacity = this.Buffer.byteLength;
        while (capacity < size) {
            capacity += capacity;
        }

        const temp = new Uint8Array(capacity);
        for (let i = 0; i < this.Length; ++i) {
            temp[i] = this.Buffer[i];
        }

        this.Buffer = temp;
        this._dview = new DataView(this.Buffer.buffer);
    }

    public Assign (buff: Uint8Array) {
        this.Buffer = buff;
        this.Length = buff.length;
        this._seekPos = buff.byteOffset;
        this._dview = new DataView(buff.buffer);
    }

    public WriteInt8 (value: number) {
        this.Reserve(this.Length + 1);

        this._dview.setInt8(this.Length, value);
        this.Length += 1;
    }

    public WriteInt16 (value: number) {
        this.Reserve(this.Length + 2);

        this._dview.setInt16(this.Length, value, true);
        this.Length += 2;
    }

    public WriteInt32 (value: number) {
        this.Reserve(this.Length + 4);

        this._dview.setInt32(this.Length, value, true);
        this.Length += 4;
    }

    public WriteIntArray (value: number[]) {
        this.Reserve(this.Length + 4 * value.length);

        for (let i = 0; i < value.length; ++i) {
            this._dview.setInt32(this.Length + i * 4, value[i], true);
        }
        this.Length += 4 * value.length;
    }

    public WriteFloat (value: number) {
        this.Reserve(this.Length + 4);

        this._dview.setFloat32(this.Length, value, true);
        this.Length += 4;
    }

    public WriteFloatArray (value: number[]) {
        this.Reserve(this.Length + 4 * value.length);

        for (let i = 0; i < value.length; ++i) {
            this._dview.setFloat32(this.Length + i * 4, value[i], true);
        }
        this.Length += 4 * value.length;
    }

    public WriteString (value: string) {
        this.Reserve(this.Length + value.length + 4);

        this._dview.setInt32(this.Length, value.length, true);
        for (let i = 0; i < value.length; ++i) {
            this._dview.setInt8(this.Length + 4 + i, value.charCodeAt(i));
        }
        this.Length += value.length + 4;
    }

    public ReadInt8 () {
        const value = this._dview.getInt8(this._seekPos);
        this._seekPos += 1;
        return value;
    }

    public ReadInt16 () {
        const value = this._dview.getInt16(this._seekPos, true);
        this._seekPos += 2;
        return value;
    }

    public ReadInt () {
        const value = this._dview.getInt32(this._seekPos, true);
        this._seekPos += 4;
        return value;
    }

    public ReadIntArray (value: number[]) {
        for (let i = 0; i < value.length; ++i) {
            value[i] = this._dview.getInt32(this._seekPos + i * 4, true);
        }
        this._seekPos += 4 * value.length;
        return value;
    }

    public ReadFloat () {
        const value = this._dview.getFloat32(this._seekPos, true);
        this._seekPos += 4;
        return value;
    }

    public ReadFloatArray (value: number[]) {
        for (let i = 0; i < value.length; ++i) {
            value[i] = this._dview.getFloat32(this._seekPos + i * 4, true);
        }
        this._seekPos += 4 * value.length;
        return value;
    }

    public ReadString () {
        const length = this.ReadInt();

        let value = '';
        for (let i = 0; i < length; ++i) {
            value += String.fromCharCode(this.ReadInt8());
        }

        return value;
    }
}

export class LFXWorld {
    public Name: string = '';
    public Settings: LFXSettings = new LFXSettings();
    public Textures: string[] = new Array<string>();
    public Terrains: LFXTerrain[] = new Array<LFXTerrain>();
    public Meshes: LFXMesh[] = new Array<LFXMesh>();
    public Lights: LFXLight[] = new Array<LFXLight>();

    public AddUniqueTexture (tex: string) {
        if (tex.length > 0) {
            for (const i of this.Textures) {
                if (i === tex) {
                    return tex;
                }
            }

            this.Textures.push(tex);
        }

        return tex;
    }
}

export class LFXTerrainLightMapInfo {
    public Id: number = 0;
    public Index: number = 0;
    public Offset: number[] = [0, 0];
    public Scale: number[] = [0, 0];
}

// tslint:disable-next-line: class-name
export class LFXMeshLightMapInfo {
    public Id: number = 0;
    public Index: number = 0;
    public Offset: number[] = [0, 0];
    public Scale: number[] = [0, 0];
}

// tslint:disable-next-line: class-name
export class LFXFile {
    public Verison: number = 0;
    public MeshInfos: LFXMeshLightMapInfo[] = new Array<LFXMeshLightMapInfo>();
    public TerrainInfos: LFXTerrainLightMapInfo[] = new Array<LFXTerrainLightMapInfo>();
}
