# 输入文件
	## Head
		int Version
		LFXSetting Settings


	## Chunks
		### Texture 0x10
			String Name
			int Width
			int Height
			int Channels
			byte *Data;

		### Terrain 0x01
			int LMapSize
			float GridSize;
			Int2 GridCount;
			int GridsPerBlock;
			float *HeightMap
			Float3 Diffuse;

		### Mesh 0x02
			String Name;
			bool CastShadow;
			bool RecieveShadow;
			int LightMapSize;

			int NumOfVertex
			int NumOfTriangle
			int NumOfMaterial

			Vertex *VertexData
				Float3 Position
				Float3 Normal
				Float2 UV
				Float2 LUV
				int MaterialId
			Triangle *TriangleData
				int Index0
				int Index1
				int Index2
				int MaterialId
			Matreial *MaterialData
				Float3 Diffuse
				String TextureName

		## Light 0x03
			int Type
			Float3 Position
			Float3 Direction
			Float3 Color
			float AttenuationStart
			float AttenuationEnd
			float AttenuationFallOff
			float SpotInner
			float SpotOuter
			float SpotFallOff
			float DirectScale
			float IndirectScale
			bool GIEnable
			bool CastShadow

# 输出文件
	## Head
		int Verison
	## Terrain
		--LightmapInfo
		--LightmapData
	## Mesh
		--Id
		--LightmapInfo
		--LightmapData
