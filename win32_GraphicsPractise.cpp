#include <math.h>
#include <windows.h>

#include "win32_GraphicsPractise.h"
#include "graphicsMath.cpp"
#include "clipper.cpp"
#include "assets.cpp"



global global_state GlobalState;
local_global v2i ScreenResolution = V2I(640, 640);

v2 NdcToPixels(v2 NdcPos)
{
	v2 Result = {};
	Result = 0.5f * (NdcPos + V2(1.0f));
	Result = Result * V2(GlobalState.FrameBufferWidth - 1, GlobalState.FrameBufferHeight - 1);
	return Result;
}

i64 CrossProduct2d(v2i A, v2i B)
{
	i64 Result = (i64(A.x) * i64(B.y)) - (i64(A.y) * i64(B.x));
	return Result;
}

v3 ColorU32ToRGB(u32 Color) 
{
	v3 Result = {};
	Result.r = (Color >> 16) & 0xFF;
	Result.g = (Color >> 8) & 0xFF;
	Result.b = (Color) & 0xFF;
	Result /= 255.0f;
	return Result;
}

v3_x4 ColorI32X4ToRGB(i32_x4 Color)
{
	v3_x4 Result = {};
	Result.b = F32X4((Color >> 16) & I32X4(0xFF));
	Result.g = F32X4((Color >> 8) & I32X4(0xFF));
	Result.r = F32X4((Color >> 0) & I32X4(0xFF));
	Result /= F32X4(255.0f);
	return Result;
}

u32 RGBToU32(v3 Color) 
{
	u32 Result = {};
	Result = ((0xFF) << 24) | (((u32)(Color.r * 255)) << 16) | (((u32)(Color.g * 255)) << 8) | ((u32)(Color.b * 255));

	return Result;
}

i32_x4 ColorRGBToI32X4(v3_x4 Color)
{
	i32_x4 Result = {};
	v3_x4 ColorTemp = F32X4(255.0f) * Color;
	Result = (I32X4(0xFF) << 24) | ((I32X4(ColorTemp.r)) << 16) | ((I32X4(ColorTemp.g)) << 8) | (I32X4(ColorTemp.b));

	return Result;
}

void DrawTriangle(clip_vertex Vertex0, clip_vertex Vertex1, clip_vertex Vertex2
	, texture Texture, sampler Sampler)
{
	Vertex0.Pos.w = 1.0f / Vertex0.Pos.w;
	Vertex1.Pos.w = 1.0f / Vertex1.Pos.w;
	Vertex2.Pos.w = 1.0f / Vertex2.Pos.w;

	Vertex0.Pos.xyz *= Vertex0.Pos.w;
	Vertex1.Pos.xyz *= Vertex1.Pos.w;
	Vertex2.Pos.xyz *= Vertex2.Pos.w;

	Vertex0.UV *= Vertex0.Pos.w;
	Vertex1.UV *= Vertex1.Pos.w;
	Vertex2.UV *= Vertex2.Pos.w;

	v2 PointAF = NdcToPixels(Vertex0.Pos.xy);
	v2 PointBF = NdcToPixels(Vertex1.Pos.xy);
	v2 PointCF = NdcToPixels(Vertex2.Pos.xy);

	i32 MinX = std::min(std::min((i32)PointAF.x, (i32)PointBF.x), (i32)PointCF.x);
	i32 MinY = std::min(std::min((i32)PointAF.y, (i32)PointBF.y), (i32)PointCF.y);
	
	i32 MaxX = max(max((i32)round(PointAF.x), (i32)round(PointBF.x)), (i32)round(PointCF.x));
	i32 MaxY = max(max((i32)round(PointAF.y), (i32)round(PointBF.y)), (i32)round(PointCF.y));

	#if 0
	MinX = max(0, MinX);
	MinX = min(MinX, GlobalState.FrameBufferWidth - 1);

	MinY = max(0, MinY);
	MinY = min(MinY, GlobalState.FrameBufferHeight - 1);

	MaxX = max(0, MaxX);
	MaxX = min(MaxX, GlobalState.FrameBufferWidth - 1);

	MaxY = max(0, MaxY);
	MaxY = min(MaxY, GlobalState.FrameBufferHeight - 1);
	#endif

	v2i PointA = V2I_F24_8(PointAF);
	v2i PointB = V2I_F24_8(PointBF);
	v2i PointC = V2I_F24_8(PointCF);


	v2i Edge0 = PointB - PointA;
	v2i Edge1 = PointC - PointB;
	v2i Edge2 = PointA - PointC;

	b32 isTopLeft0 = (Edge0.y > 0 || Edge0.x > 0 && Edge0.y == 0);
	b32 isTopLeft1 = (Edge1.y > 0 || Edge1.x > 0 && Edge1.y == 0);
	b32 isTopLeft2 = (Edge2.y > 0 || Edge2.x > 0 && Edge2.y == 0);

	f32_x4 BaryCentricDiv = F32X4(256.0f / f32(CrossProduct2d(PointB - PointA, PointC - PointA)));

	i32_x4 Edge0DiffX = I32X4(Edge0.y);
	i32_x4 Edge0DiffY = I32X4(-Edge0.x);
	i32_x4 Edge1DiffX = I32X4(Edge1.y);
	i32_x4 Edge1DiffY = I32X4(-Edge1.x);
	i32_x4 Edge2DiffX = I32X4(Edge2.y);
	i32_x4 Edge2DiffY = I32X4(-Edge2.x);

	i32_x4 Edge0RowY = {};
	i32_x4 Edge1RowY = {};
	i32_x4 Edge2RowY = {};

	{
		v2i StartPos = V2I_F24_8(V2(MinX, MinY) + V2(0.5f, 0.5f));

		i64 Edge0RowY64 = CrossProduct2d(StartPos - PointA, Edge0);
		i64 Edge1RowY64 = CrossProduct2d(StartPos - PointB, Edge1);
		i64 Edge2RowY64 = CrossProduct2d(StartPos - PointC, Edge2);

		i32 Edge0RowY32 = i32((Edge0RowY64 + Sign(Edge0RowY64) * 128) / 256) - (isTopLeft0 ? 0 : -1);
		i32 Edge1RowY32 = i32((Edge1RowY64 + Sign(Edge1RowY64) * 128) / 256) - (isTopLeft1 ? 0 : -1);
		i32 Edge2RowY32 = i32((Edge2RowY64 + Sign(Edge2RowY64) * 128) / 256) - (isTopLeft2 ? 0 : -1);

		Edge0RowY = I32X4(Edge0RowY32) + (I32X4(0, 1, 2, 3) * Edge0DiffX);
		Edge1RowY = I32X4(Edge1RowY32) + (I32X4(0, 1, 2, 3) * Edge1DiffX);
		Edge2RowY = I32X4(Edge2RowY32) + (I32X4(0, 1, 2, 3) * Edge2DiffX);
	}
	Edge0DiffX = Edge0DiffX * I32X4(4);
	Edge1DiffX = Edge1DiffX * I32X4(4);
	Edge2DiffX = Edge2DiffX * I32X4(4);


	for (int Y = { MinY }; Y <= MaxY; ++Y)
	{
		i32_x4 Edge0RowX = Edge0RowY;
		i32_x4 Edge1RowX = Edge1RowY;
		i32_x4 Edge2RowX = Edge2RowY;

		for (int X = { MinX }; X <= MaxX; X += 4)
		{
			u32 PixelID = (Y * GlobalState.FrameBufferStride) + X;

			f32* DepthPtr = GlobalState.DepthBuffer + PixelID;
			f32_x4 PixelDepths = F32X4Load(DepthPtr);

			i32* ColorPtr = (i32*)GlobalState.FrameBufferPixels + PixelID;
			i32_x4 PixelColors = I32X4Load(ColorPtr);

			i32_x4 EdgeMask = (Edge0RowX | Edge1RowX | Edge2RowX) >= 0;
			if (_mm_movemask_epi8(EdgeMask.Vals)) {

				f32_x4 T0 = -F32X4(Edge1RowX) * BaryCentricDiv;
				f32_x4 T1 = -F32X4(Edge2RowX) * BaryCentricDiv;
				f32_x4 T2 = -F32X4(Edge0RowX) * BaryCentricDiv;

				f32_x4 Depth = Vertex0.Pos.z + T1 * (Vertex1.Pos.z - Vertex0.Pos.z) + T2 * (Vertex2.Pos.z - Vertex0.Pos.z);
				i32_x4 DepthMask = I32X4ReInterpret(Depth < PixelDepths);


				f32_x4 OneOverW = T0 * Vertex0.Pos.w + T1 * Vertex1.Pos.w + T2 * Vertex2.Pos.w;

				v2_x4 UV = T0 * V2X4(Vertex0.UV) + T1 * V2X4(Vertex1.UV) + T2 * V2X4(Vertex2.UV);
				UV /= OneOverW;

				i32_x4 TextureColor = I32X4(0);

				switch (Sampler.Type)
				{
					case(SamplerType_Nearest):
					{
						i32_x4 TexelX = I32X4(Floor(UV.x * (Texture.Width - 1)));
						i32_x4 TexelY = I32X4(Floor(UV.y * (Texture.Height - 1)));

						TexelX = Max(Min(TexelX, I32X4(Texture.Width - 1)), I32X4(0));
						TexelY = Max(Min(TexelY, I32X4(Texture.Height - 1)), I32X4(0));

						i32_x4 TexelMask = (TexelX >= 0 & TexelX < Texture.Width
							& TexelY >= 0 & TexelY < Texture.Height);

						i32_x4 TexelOffsets = TexelY * I32X4(Texture.Width) + TexelX;

						i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
						i32_x4 FalseCase = I32X4((i32)0xffff00ff);

						TextureColor = (TexelMask & TrueCase) + AndNot(TexelMask, FalseCase);
					} break;
					case(SamplerType_Bilinear):
					{
#if 0
						v2_x4 TexelV2 = UV * V2X4(Texture.Width, Texture.Height) - V2X4(0.5f, 0.5f);
						v2i_x4 TexelPos[4] = {};
						TexelPos[0] = V2IX4(Floor(TexelV2.x), Floor(TexelV2.y));
						TexelPos[1] = TexelPos[0] + V2IX4(1, 0);
						TexelPos[2] = TexelPos[0] + V2IX4(0, 1);
						TexelPos[3] = TexelPos[0] + V2IX4(1, 1);

						v3_x4 TexelColors[4] = {};

						for (u32 TexelID = { 0 }; TexelID < ArrayCount(TexelPos); TexelID++)
						{
							v2i_x4 CurrTexelPos = TexelPos[TexelID];
							i32_x4 TexelMask = (CurrTexelPos.x >= 0 & CurrTexelPos.x < Texture.Width
								& CurrTexelPos.y >= 0 & CurrTexelPos.y < Texture.Height);

							CurrTexelPos.x = Max(Min(CurrTexelPos.x, I32X4(Texture.Width - 1)), I32X4(0));
							CurrTexelPos.y = Max(Min(CurrTexelPos.y, I32X4(Texture.Height - 1)), I32X4(0));

							i32_x4 TexelOffsets = CurrTexelPos.y * I32X4(Texture.Width) + CurrTexelPos.x;
							i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
							i32_x4 FalseCase = I32X4((i32)Sampler.BoarderColor);
							i32_x4 TexelColorsI32 = (TexelMask & TrueCase) + AndNot(TexelMask, FalseCase);

							TexelColors[TexelID] = ColorI32X4ToRGB(TexelColorsI32);
						}
						f32_x4 S = TexelV2.x - Floor(TexelV2.x);
						f32_x4 K = TexelV2.y - Floor(TexelV2.y);

						v3_x4 Interpolated0 = Lerp(TexelColors[0], TexelColors[1], S);
						v3_x4 Interpolated1 = Lerp(TexelColors[2], TexelColors[3], S);
						v3_x4 FinalColor = Lerp(Interpolated0, Interpolated1, K);

						TextureColor = ColorRGBToI32X4(FinalColor);
#else
						v2_x4 TexelV2 = UV * V2X4(Texture.Width, Texture.Height) - V2X4(0.5f, 0.5f);
						v2i_x4 TexelPos[4] = {};
						TexelPos[0] = V2IX4(Floor(TexelV2.x), Floor(TexelV2.y));
						TexelPos[1] = TexelPos[0] + V2IX4(1, 0);
						TexelPos[2] = TexelPos[0] + V2IX4(0, 1);
						TexelPos[3] = TexelPos[0] + V2IX4(1, 1);

						v3_x4 TexelColors[4] = {};

						for (u32 TexelID = { 0 }; TexelID < ArrayCount(TexelPos); TexelID++)
						{
							v2i_x4 CurrTexelPos = TexelPos[TexelID];
							{
								v2_x4 CurrTexelPosF = V2X4(CurrTexelPos);
								v2_x4 Factor = {};
									Factor.x = Floor(CurrTexelPosF.x / F32X4(Texture.Width));
									Factor.y = Floor(CurrTexelPosF.y / F32X4(Texture.Height));
								CurrTexelPosF.x = CurrTexelPosF.x - (F32X4(Texture.Width) * Factor.x);
								CurrTexelPosF.y = CurrTexelPosF.y - (F32X4(Texture.Height) * Factor.y);
								CurrTexelPos = V2IX4(CurrTexelPosF);
							}

							i32_x4 TexelOffsets = CurrTexelPos.y * I32X4(Texture.Width) + CurrTexelPos.x;
							i32_x4 LoadMask = EdgeMask & DepthMask;

							TexelOffsets = (TexelOffsets & LoadMask) + AndNot(LoadMask, I32X4(0));

							i32_x4 TexelColorsI32 = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
							
							TexelColors[TexelID] = ColorI32X4ToRGB(TexelColorsI32);
						}
						f32_x4 S = TexelV2.x - Floor(TexelV2.x);
						f32_x4 K = TexelV2.y - Floor(TexelV2.y);

						v3_x4 Interpolated0 = Lerp(TexelColors[0], TexelColors[1], S);
						v3_x4 Interpolated1 = Lerp(TexelColors[2], TexelColors[3], S);
						v3_x4 FinalColor = Lerp(Interpolated0, Interpolated1, K);

						TextureColor = ColorRGBToI32X4(FinalColor);
#endif 
					} break;
					default:
					{
						InvalidCodePath;
					} break;
				}

				i32_x4 FinalMaskI32 = EdgeMask & DepthMask;
				f32_x4 FinalMaskF32 = F32X4ReInterpret(FinalMaskI32);

				i32_x4 OutputColors = (TextureColor & FinalMaskI32) + AndNot(FinalMaskI32, PixelColors);
				f32_x4 OutputDepth = (Depth & FinalMaskF32) + AndNot(FinalMaskF32, PixelDepths);

				I32X4Store(ColorPtr, OutputColors);
				F32X4Store(DepthPtr, OutputDepth);
			}
			Edge0RowX += Edge0DiffX;
			Edge1RowX += Edge1DiffX;
			Edge2RowX += Edge2DiffX;
		}
		Edge0RowY += Edge0DiffY;
		Edge1RowY += Edge1DiffY;
		Edge2RowY += Edge2DiffY;
	}
}

void DrawTriangle(v4 ModelVertex0, v4 ModelVertex1, v4 ModelVertex2
	, v2 ModelUV0, v2 ModelUV1, v2 ModelUV2
	, texture Texture, sampler Sampler)
{

	clip_result Ping;
	Ping.NumTriangles = 1;
	Ping.Vertices[0] = { ModelVertex0, ModelUV0 };
	Ping.Vertices[1] = { ModelVertex1, ModelUV1 };
	Ping.Vertices[2] = { ModelVertex2, ModelUV2 };

	clip_result Pong = {};

	ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Left);
	ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Right);
	ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Top);
	ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Bottom);
	ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Near);
	ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Far);
	ClipPolygonToAxis(&Ping, &Pong, ClipAxis_W);

	for (int TriangleId = 0; TriangleId < Pong.NumTriangles; ++TriangleId) 
	{
		DrawTriangle(Pong.Vertices[3 * TriangleId + 0], Pong.Vertices[3 * TriangleId + 1],
			Pong.Vertices[3 * TriangleId + 2], Texture, Sampler);
	}
}

LRESULT Win32WindowCallBack(
	HWND WindowHandle,
	UINT Massage,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = {};

	switch (Massage)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{ GlobalState.IsRunning = false; } break;

	default:
	{ Result = DefWindowProcA(WindowHandle, Massage, WParam, LParam); } break;

	}
	return Result;
};

void DrawModel(model* Model, m4 Transform, sampler Sampler)
{
	v4* TransformedVertices = (v4*)malloc(sizeof(v4) * Model->VertexCount);
	for (u32 VertexId = { 0 }; VertexId < Model->VertexCount; ++VertexId)
	{
		TransformedVertices[VertexId] = (Transform * V4(Model->VertexArray[VertexId].Pos, 1.0f));
	}
	
	for (u32 MeshID = 0; MeshID < Model->NumMeshes; ++MeshID) 
	{
		mesh* CurrentMesh = Model->MeshArray + MeshID;
		for (u32 IndexId = 0; IndexId < CurrentMesh->IndexCount; IndexId += 3)
		{	
			u32 Index0 = Model->IndexArray[CurrentMesh->IndexOffset + IndexId + 0];
			u32 Index1 = Model->IndexArray[CurrentMesh->IndexOffset + IndexId + 1];
			u32 Index2 = Model->IndexArray[CurrentMesh->IndexOffset + IndexId + 2];

			v4 Pos0 = TransformedVertices[CurrentMesh->VertexOffset + Index0];
			v4 Pos1 = TransformedVertices[CurrentMesh->VertexOffset + Index1];
			v4 Pos2 = TransformedVertices[CurrentMesh->VertexOffset + Index2];

			v2 UV0 = Model->VertexArray[CurrentMesh->VertexOffset + Index0].UV;
			v2 UV1 = Model->VertexArray[CurrentMesh->VertexOffset + Index1].UV;
			v2 UV2 = Model->VertexArray[CurrentMesh->VertexOffset + Index2].UV;

			DrawTriangle(Pos0, Pos1, Pos2, UV0, UV1, UV2, Model->TextureArray[CurrentMesh->TextureIndex], Sampler);
		}
	}

	free(TransformedVertices);
}


int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd)
{
	GlobalState.IsRunning = true;

	LARGE_INTEGER timerFrequency = {};
	Assert(QueryPerformanceFrequency(&timerFrequency));



	{

		WNDCLASSA windowClass = {};
		windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = Win32WindowCallBack;
		windowClass.hInstance = hInstance;
		windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
		windowClass.lpszClassName = "PractisingInGraphicsPart2";

		if (!RegisterClassA(&windowClass)) { InvalidCodePath; }

		GlobalState.WindowHandle = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"OneCoreRender",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenResolution.x, ScreenResolution.y,
			NULL,
			NULL,
			hInstance,
			NULL);

		if (!GlobalState.WindowHandle) { InvalidCodePath; }

		GlobalState.DeviceContext = GetDC(GlobalState.WindowHandle);
		if (!GlobalState.DeviceContext) { InvalidCodePath; }
	}

	{
		RECT clientRect = {};
		Assert(GetClientRect(GlobalState.WindowHandle, &clientRect));

		GlobalState.FrameBufferWidth = clientRect.right - clientRect.left;
		GlobalState.FrameBufferHeight = clientRect.bottom - clientRect.top;
		GlobalState.FrameBufferStride = GlobalState.FrameBufferWidth + 3;

		GlobalState.FrameBufferPixels = (u32*)malloc(sizeof(u32) * GlobalState.FrameBufferStride * GlobalState.FrameBufferHeight);
		GlobalState.DepthBuffer = (f32*)malloc(sizeof(f32) * GlobalState.FrameBufferStride * GlobalState.FrameBufferHeight);
	}
	
	texture CheckerBoardTexture = {};
	sampler Sampler = {};
	{
		Sampler.Type = SamplerType_Bilinear;
		Sampler.BoarderColor = 6;

		u32 Blocksize = 4;
		u32 numBlocks = 32;

		CheckerBoardTexture.Width = Blocksize * numBlocks;
		CheckerBoardTexture.Height = Blocksize * numBlocks;

		CheckerBoardTexture.Texels = (u32*)malloc(sizeof(u32*) * CheckerBoardTexture.Width * CheckerBoardTexture.Height);
		for (int Y = { 0 }; Y < numBlocks; Y++)
		{
			for (int X = { 0 }; X < numBlocks; X++)
			{
				u32 ColorChannelR = 0xBF * ((X + (Y % 2)) % 2);
				u32 ColorChannelG = 0x40 * ((X + (Y % 2)) % 2);
				u32 ColorChannelB = 0xBF * ((X + (Y % 2)) % 2);
				
				for (u32 BlockY = 0; BlockY < Blocksize; BlockY++)
				{
					for (u32 BlockX = 0; BlockX < Blocksize; BlockX++)
					{
						u32 TexelID = (Y * Blocksize + BlockY) * CheckerBoardTexture.Width + (X * Blocksize + BlockX);
						CheckerBoardTexture.Texels[TexelID] = (((u32)0xFF) << 24) + (((u32)ColorChannelR) << 16) + (((u32)ColorChannelG) << 8) + ((u32)ColorChannelB);
					}
				}
			}
		}
	}

	{
		// CubeModel
		local_global vertex ModelVertices[] = {
			//FrontFace
			{ V3(-0.5f, -0.5f, -0.5f), V2(0, 0) },
			{ V3(-0.5f, 0.5f, -0.5f), V2(0, 1) },
			{ V3(0.5f, 0.5f, -0.5f), V2(1, 1) },
			{ V3(0.5f, -0.5f, -0.5f), V2(1, 0) },

			//BackFace
			{ V3(-0.5f, -0.5f, 0.5f), V2(1, 0) },
			{ V3(-0.5f, 0.5f, 0.5f), V2(1, 1) },
			{ V3(0.5f, 0.5f, 0.5f), V2(0, 1) },
			{ V3(0.5f, -0.5f, 0.5f), V2(0, 0) }
		};

		local_global u32 ModelIndices[] =
		{
			//FrontFace
			0, 1, 2,
			2, 3, 0,
			//Backface
			6, 5, 4,
			4, 7, 6,
			//LeftFace
			4, 5, 1,
			1, 0, 4,
			//RightFace
			3, 2, 6,
			6 ,7 ,3,
			//TopFace
			1, 5, 6,
			6, 2, 1,
			//BottomFace
			4, 0, 3,
			3, 7, 4
		};

		local_global mesh Mesh = {};
		Mesh.TextureIndex = 0;

		Mesh.IndexOffset = 0;
		Mesh.IndexCount = ArrayCount(ModelIndices);

		Mesh.VertexOffset = 0;
		Mesh.VertexCount = ArrayCount(ModelVertices);

		GlobalState.CubeModel = {};

		GlobalState.CubeModel.NumMeshes = 1;
		GlobalState.CubeModel.MeshArray = &Mesh;

		GlobalState.CubeModel.VertexCount = ArrayCount(ModelVertices);
		GlobalState.CubeModel.VertexArray = ModelVertices;

		GlobalState.CubeModel.IndexCount = ArrayCount(ModelIndices);
		GlobalState.CubeModel.IndexArray = ModelIndices;
		
		GlobalState.CubeModel.NumTextures = 1;
		GlobalState.CubeModel.TextureArray = &CheckerBoardTexture;
	}

	{
		// HourGlassModel
		local_global vertex ModelVertices[] = {
			//Top
			{ V3(0.25f, 0.5f, 0.25f), V2(0, 0) },
			{ V3(0.25f, 0.5f, -0.25f), V2(0, 1) },
			{ V3(-0.25f, 0.5f, -0.25f), V2(1, 1) },
			{ V3(-0.25f, 0.5f, 0.25f), V2(1, 0) },

			//Middle
			{ V3(0.0f, 0.0f, 0.0f), V2(0, 0) },

			//Bottom
			{ V3(0.25f, -0.5f, 0.25f), V2(0, 1) },
			{ V3(0.25f, -0.5f, -0.25f), V2(1, 1) },
			{ V3(-0.25f, -0.5f, -0.25f), V2(0, 1) },
			{ V3(-0.25f, -0.5f, 0.25f), V2(0, 0) }
		};

		local_global u32 ModelIndices[] =
		{
			//FrontFace
			4, 0, 3,
			4, 8, 5,
			//Backface
			4, 2, 1,
			4, 6, 7,
			//LeftFace
			4, 1, 0,
			4, 5, 6,
			//RightFace
			4, 3, 2,
			4 ,7 ,8,
			//TopFace
			0, 1, 3,
			1, 2, 3,
			//BottomFace
			6, 8, 7,
			5, 8, 6
		};

		local_global mesh Mesh = {};
		Mesh.TextureIndex = 0;

		Mesh.IndexOffset = 0;
		Mesh.IndexCount = ArrayCount(ModelIndices);

		Mesh.VertexOffset = 0;
		Mesh.VertexCount = ArrayCount(ModelVertices);


		GlobalState.HourGlassModel = {};

		GlobalState.HourGlassModel.NumMeshes = 1;
		GlobalState.HourGlassModel.MeshArray = &Mesh;

		GlobalState.HourGlassModel.VertexCount = ArrayCount(ModelVertices);
		GlobalState.HourGlassModel.VertexArray = ModelVertices;

		GlobalState.HourGlassModel.IndexCount = ArrayCount(ModelIndices);
		GlobalState.HourGlassModel.IndexArray = ModelIndices;

		GlobalState.HourGlassModel.NumTextures = 1;
		GlobalState.HourGlassModel.TextureArray = &CheckerBoardTexture;
	}

	GlobalState.DuckModel = AssetLoadModel((char*)"../data/Duck/", (char*)"Duck.gltf");
	GlobalState.SponzaModel = AssetLoadModel((char*)"../data/Sponza/", (char*)"Sponza.gltf");
	
	LARGE_INTEGER beginTime = {};
	LARGE_INTEGER endTime = {};

	Assert(QueryPerformanceCounter(&beginTime));

	while (GlobalState.IsRunning)
	{
		Assert(QueryPerformanceCounter(&endTime));
		f32 frameTime = f32(endTime.QuadPart - beginTime.QuadPart) / f32(timerFrequency.QuadPart);
		beginTime = endTime;

		{
			char Text[256];
			snprintf(Text, sizeof(Text), "FrameTime: %f\n", frameTime);
			OutputDebugStringA(Text);
		}

		MSG Message = {};
		while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
		{
			switch (Message.message)
			{
			case WM_QUIT:
			{ GlobalState.IsRunning = false;
			} break;
			case WM_KEYUP:
			case WM_KEYDOWN:
			{
				u32 VkCode = Message.wParam;
				b32 IsDown = !(Message.lParam >> 31) & 0x1;
				switch (VkCode) 
				{
					case 'W':
					{
						GlobalState.WDown = IsDown;
					} break;
					case 'A':
					{
						GlobalState.ADown = IsDown;
					} break;
					case 'S':
					{
						GlobalState.SDown = IsDown;
					} break;
					case 'D':
					{
						GlobalState.DDown = IsDown;
					} break;
					case 'J':
					{
						GlobalState.Camera.Pos = V3(0, 0, 0);
						GlobalState.Camera.Pitch = 0;
						GlobalState.Camera.Yaw = 0;
					} break;
				}
			} break;

			default:
			{ TranslateMessage(&Message);
			DispatchMessage(&Message);
			} break;
			}
		}

		GlobalState.CurrTime += frameTime;

		for (u32 Y = 0; Y < GlobalState.FrameBufferHeight; ++Y)
		{
			for (u32 X = 0; X < GlobalState.FrameBufferWidth; ++X)
			{
				u32 pixelID = (Y * GlobalState.FrameBufferStride) + X;
				GlobalState.FrameBufferPixels[pixelID] = 0xFF000000;
				GlobalState.DepthBuffer[pixelID] = FLT_MAX;
			}
		}

		f32 rotationSpeed = f32(1.0f);
		GlobalState.CurrRot += frameTime * rotationSpeed;
		if (GlobalState.CurrRot > 2.0f * 3.14159f)
		{
			GlobalState.CurrRot -= 2.0f * 3.14159f;
		}

		RECT clientRect = {};
		Assert(GetClientRect(GlobalState.WindowHandle, &clientRect));
		u32 clientWidth = clientRect.right - clientRect.left;
		u32 clientHeight = clientRect.bottom - clientRect.top;

		f32 AspectRatio = f32(clientWidth) / f32(clientHeight);

		m4 CameraTransform = IdentityM4();
		{
			camera* Camera = &GlobalState.Camera;

			b32 MouseDown = false;
			v2 CurrMousePos = {};
			if(GetActiveWindow() == GlobalState.WindowHandle)
			{
				POINT Win32MousePos = {};
				Assert(GetCursorPos(&Win32MousePos));
				Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));

				Win32MousePos.y = clientRect.bottom - Win32MousePos.y;

				CurrMousePos.x = f32(Win32MousePos.x) / f32(clientWidth);
				CurrMousePos.y = f32(Win32MousePos.y) / f32(clientHeight);

				MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;

			}
			
			if(MouseDown)
			{
				if (!Camera->PrevMouseDown)
				{
					Camera->PrevMousePos = CurrMousePos;
				}

				v2 MouseDelta = CurrMousePos - Camera->PrevMousePos;
				Camera->Yaw += MouseDelta.x;
				Camera->Pitch += MouseDelta.y;

				Camera->PrevMousePos = CurrMousePos;
			}

			Camera->PrevMouseDown = MouseDown;

			m4 YawTransform = RotationMatrix(0, Camera->Yaw, 0);
			m4 PitchTransform = RotationMatrix(Camera->Pitch, 0, 0);
			m4 CameraAxisTransform = YawTransform * PitchTransform;
			
			v3 Right = Normalize((CameraAxisTransform * V4(1, 0, 0, 0)).xyz);
			v3 Up = Normalize((CameraAxisTransform * V4(0, 1, 0, 0)).xyz);
			v3 LookAt = Normalize((CameraAxisTransform * V4(0, 0, 1, 0)).xyz);

			m4 CameraViewTransform = IdentityM4();

			CameraViewTransform.v[0].x = Right.x;
			CameraViewTransform.v[1].x = Right.y;
			CameraViewTransform.v[2].x = Right.z;

			CameraViewTransform.v[0].y = Up.x;
			CameraViewTransform.v[1].y = Up.y;
			CameraViewTransform.v[2].y = Up.z;

			CameraViewTransform.v[0].z = LookAt.x;
			CameraViewTransform.v[1].z = LookAt.y;
			CameraViewTransform.v[2].z = LookAt.z;

			if (GlobalState.WDown)
			{
				Camera->Pos += LookAt * frameTime;
			}
			if (GlobalState.ADown)
			{
				Camera->Pos -= Right * frameTime;
			}
			if (GlobalState.SDown)
			{
				Camera->Pos -= LookAt * frameTime;
			}
			if (GlobalState.DDown)
			{
				Camera->Pos += Right * frameTime;
			}


			CameraTransform = CameraViewTransform * TranslationMatrix(-Camera->Pos);
		}

		f32 Offset = abs(sin(GlobalState.CurrRot));
		m4 Transform = PerspectiveMatrix(90.0f, AspectRatio, 0.01f, 1000.0f)
			* CameraTransform
			* TranslationMatrix(0, 0, 2)
			* RotationMatrix(GlobalState.CurrTime * 2, GlobalState.CurrTime, GlobalState.CurrTime / 2)
			* ScaleMatrix(1, 1, 1);

		DrawModel(&GlobalState.HourGlassModel, Transform, Sampler);
		
		BITMAPINFO BitmapInfo = {};
		BitmapInfo.bmiHeader = {};
		BitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
		BitmapInfo.bmiHeader.biWidth = GlobalState.FrameBufferStride;
		BitmapInfo.bmiHeader.biHeight = GlobalState.FrameBufferHeight;
		BitmapInfo.bmiHeader.biPlanes = 1;
		BitmapInfo.bmiHeader.biBitCount = 32;
		BitmapInfo.bmiHeader.biCompression = BI_RGB;

		Assert(StretchDIBits(
			GlobalState.DeviceContext,
			0,
			0,
			clientWidth,
			clientHeight,
			0,
			0,
			GlobalState.FrameBufferWidth,
			GlobalState.FrameBufferHeight,
			GlobalState.FrameBufferPixels,
			&BitmapInfo,
			DIB_RGB_COLORS,
			SRCCOPY));
	}
	return 0;
}