//-----------------------------------------------------------------------------
// File: Lights.cpp
//
// Desc: Rendering 3D geometry is much more interesting when dynamic lighting
//       is added to the scene. To use lighting in D3D, you must create one or
//       lights, setup a material, and make sure your geometry contains surface
//       normals. Lights may have a position, a color, and be of a certain type
//       such as directional (light comes from one direction), point (light
//       comes from a specific x,y,z coordinate and radiates in all directions)
//       or spotlight. Materials describe the surface of your geometry,
//       specifically, how it gets lit (diffuse color, ambient color, etc.).
//       Surface normals are part of a vertex, and are needed for the D3D's
//       internal lighting calculations.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#include<time.h>
#pragma warning( default : 4996 )




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices

// A structure for our custom vertex type. We added a normal, and omitted the
// color (which is provided by the material)
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The 3D position for the vertex
	D3DXVECTOR3 normal;   // The surface normal for the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

int iTime;
FLOAT fAngle;

ID3DXMesh* solarSystemObjects[]{ 0,0,0,0,0,0,0,0,0 };
ID3DXMesh* satelliteObjects[]{ 0,0,0 };
ID3DXMesh* spacecraftObject;

//0.6,0.5,0.4
FLOAT satelliteRadius[]{ 0.25f,0.6f,0.5f };
FLOAT radius[]{ 5.0f,0.3f,0.8f,1.0f,1.2f,3.0f,2.5f,1.8f,1.7f };

//행성 태양은 행성이아니긴한데.. 대충 태 수 금 지 화 목 토 천 해
enum Planet { SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE };

//위성 달,타이탄
enum Satellite { MOON, TITAN, IO };

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Create the D3D object.
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// Set up the structure used to create the D3DDevice. Since we are now
	// using more complex geometry, we will create a device with a zbuffer.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// Create the D3DDevice
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// Turn off culling
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Turn on the zbuffer
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// Create the vertex buffer.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(50 * 2 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Fill the vertex buffer. We are algorithmically generating a cylinder
	// here, including the normals, which are used for lighting.
	CUSTOMVERTEX* pVertices;
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	for (int i = 0; i < 9; i++)
	{
		D3DXCreateSphere(g_pd3dDevice, radius[i], 20, 20, &solarSystemObjects[i], 0);
	}

	for (int i = 0; i < 3; i++)
	{
		D3DXCreateSphere(g_pd3dDevice, satelliteRadius[i], 20, 20, &satelliteObjects[i], 0);
	}

	D3DXCreateBox(g_pd3dDevice, 3, 3, 3, &spacecraftObject, NULL);

	g_pVB->Unlock();



	/*for (int i = 0; i < 8; i++)
	{
		D3DXCreateSphere(g_pd3dDevice, radius[i], 20, 20, &solarSystemObjects[i], 0);
	}*/

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();

	for (int i = 0; i < 9; i++)
	{
		solarSystemObjects[i]->Release();
	}

	for (int i = 0; i < 3; i++)
	{
		satelliteObjects[i]->Release();
	}
	spacecraftObject->Release();
}

//공전 시스템
VOID RotateRevolveSystem(D3DXMATRIX& rot, Planet planet)
{
	switch (planet)
	{
	case SUN:
		//iTime = timeGetTime() % 100000;
		//fAngle = iTime * (2.0f * D3DX_PI) / 100000.0f;
		break;
	case MERCURY:
		iTime = timeGetTime() % 2000;
		fAngle = iTime * (2.0f * D3DX_PI) / 2000.0f;
		break;
	case VENUS:
		iTime = timeGetTime() % 6000;
		fAngle = iTime * (2.0f * D3DX_PI) / 6000.0f;
		break;
	case EARTH:
		iTime = timeGetTime() % 10000;
		fAngle = iTime * (2.0f * D3DX_PI) / 10000.0f;
		break;
	case MARS:
		iTime = timeGetTime() % 18000;
		fAngle = iTime * (2.0f * D3DX_PI) / 18000.0f;
		break;
	case JUPITER:
		iTime = timeGetTime() % 50000;
		fAngle = iTime * (2.0f * D3DX_PI) / 50000.0f;
		break;
	case SATURN:
		iTime = timeGetTime() % 100000;
		fAngle = iTime * (2.0f * D3DX_PI) / 100000.0f;
		break;
	case URANUS:
		iTime = timeGetTime() % 200000;
		fAngle = iTime * (2.0f * D3DX_PI) / 200000.0f;
		break;
	case NEPTUNE:
		iTime = timeGetTime() % 300000;
		fAngle = iTime * (2.0f * D3DX_PI) / 300000.0f;
		break;
	}

	D3DXMatrixRotationY(&rot, fAngle);

}

//자전
VOID SelfRotateSystem(D3DXMATRIX& rot, Planet planet)
{
	switch (planet)
	{
	case SUN:
		iTime = timeGetTime() % 100000;
		fAngle = iTime * (2.0f * D3DX_PI) / 100000.0f;
		break;
	case MERCURY:
		iTime = timeGetTime() % 200000;
		fAngle = iTime * (2.0f * D3DX_PI) / 200000.0f;
		break;
	case VENUS:
		iTime = timeGetTime() % 240000;
		fAngle = iTime * (2.0f * D3DX_PI) / 240000.0f * (-1);
		break;
	case EARTH:
		iTime = timeGetTime() % 10000;
		fAngle = iTime * (2.0f * D3DX_PI) / 10000.0f;
		break;
	case MARS:
		iTime = timeGetTime() % 11000;
		fAngle = iTime * (2.0f * D3DX_PI) / 11000.0f;
		break;
	case JUPITER:
		iTime = timeGetTime() % 3000;
		fAngle = iTime * (2.0f * D3DX_PI) / 3000.0f;
		break;
	case SATURN:
		iTime = timeGetTime() % 3500;
		fAngle = iTime * (2.0f * D3DX_PI) / 3500.0f;
		break;
	case URANUS:
		iTime = timeGetTime() % 8000;
		fAngle = iTime * (2.0f * D3DX_PI) / 8000.0f * (-1);
		D3DXMatrixRotationX(&rot, fAngle);
		break;
	case NEPTUNE:
		iTime = timeGetTime() % 8000;
		fAngle = iTime * (2.0f * D3DX_PI) / 8000.0f;
		break;
	}

	if (planet != URANUS)
		D3DXMatrixRotationY(&rot, fAngle);


}

//위성공전
VOID RotateRevolveSystem(D3DXMATRIX& rot, Satellite satellite)
{

	switch (satellite)
	{
	case MOON:
		iTime = timeGetTime() % 30000;
		fAngle = iTime * (2.0f * D3DX_PI) / 30000.0f;
		break;
	case TITAN:
		iTime = timeGetTime() % 15000;
		fAngle = iTime * (2.0f * D3DX_PI) / 15000.0f;
		break;
	case IO:
		iTime = timeGetTime() % 10000;
		fAngle = iTime * (2.0f * D3DX_PI) / 10000.0f;
		break;
	}

	D3DXMatrixRotationY(&rot, fAngle);

}


//태양계 행성들구성해주기
VOID SolarSystem()
{
	//태양계에 들어갈 오브젝트들 0번 태양 1번지구 2번 달 3번 화성..

	D3DMATERIAL9 Mtrls[]{ 0,0,0,0,0,0,0,0,0 };

	//행성 행렬값
	D3DXMATRIX Worlds[9];

	//태0 수1 금2 지3 화4 목5 토6 천7 해8 
	//행성
	FLOAT xPoints[]{ 0.0f,8.5f,10.0f,15.0f,20.0f,27.0f,33.0f,37.0f,40.0f };
	FLOAT yPoints[]{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
	FLOAT zPoints[]{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };

	//위성들 행렬값
	D3DXMATRIX satelliteWorlds[3];

	FLOAT xPoints_satel[]{ 3.0f,5.0f,5.0f };
	FLOAT yPoints_satel[]{ 0.f,0.f,0.f };
	FLOAT zPoints_satel[]{ 0.f,0.f,0.f };


	//돌려돌려 행성판~~ 행성돌려버리기~
	//행성공전


	for (int i = 0; i < 9; i++)
	{
		D3DXMatrixIdentity(&Worlds[i]);
		D3DXMATRIXA16 matTrans;
		D3DXMATRIXA16 matRot;

		D3DXMatrixTranslation(&matTrans, xPoints[i], yPoints[i], zPoints[i]);
		RotateRevolveSystem(matRot, (Planet)i);

		Worlds[i] = matTrans * matRot * Worlds[i];

	}
	D3DXMATRIXA16 parent;

	//행성자전
	for (int i = 0; i < 9; i++)
	{
		//D3DXMatrixIdentity(&Worlds[i]);
		D3DXMATRIXA16 matTrans;
		D3DXMATRIXA16 matRot;

		SelfRotateSystem(matRot, (Planet)i);
		D3DXMatrixTranslation(&matTrans, xPoints[i], yPoints[i], zPoints[i]);

		Worlds[i] = matRot * matTrans * Worlds[i];

		//위성들이 공전,자전 주기가 같아서 공전 하나로처리하였음..

		if (i == EARTH)	//달
		{
			D3DXMATRIXA16 sateRot;
			D3DXMATRIXA16 sateTrans;

			//달은 지구를 중심으로 공전
			D3DXMatrixTranslation(&sateTrans, xPoints_satel[0], yPoints_satel[0], zPoints_satel[0]);
			RotateRevolveSystem(sateRot, (Satellite)0);

			satelliteWorlds[0] = sateTrans * sateRot * Worlds[i];
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &satelliteWorlds[0]);
			satelliteObjects[0]->DrawSubset(0);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &Worlds[i]);
			solarSystemObjects[i]->DrawSubset(0);
		}
		else if (i == JUPITER)		//이오
		{
			D3DXMATRIXA16 sateRot;
			D3DXMATRIXA16 sateTrans;

			//이오은 목성을 중심으로 공전
			D3DXMatrixTranslation(&sateTrans, xPoints_satel[2], yPoints_satel[2], zPoints_satel[2]);
			RotateRevolveSystem(sateRot, (Satellite)2);

			satelliteWorlds[2] = sateTrans * sateRot * Worlds[i];
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &satelliteWorlds[2]);
			satelliteObjects[2]->DrawSubset(0);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &Worlds[i]);
			solarSystemObjects[i]->DrawSubset(0);
		}
		else if (i == SATURN)		//타이탄
		{
			D3DXMATRIXA16 sateRot;
			D3DXMATRIXA16 sateTrans;

			//타이탄은 토성을 중심으로 공전한다.
			D3DXMatrixTranslation(&sateTrans, xPoints_satel[1], yPoints_satel[1], zPoints_satel[1]);
			RotateRevolveSystem(sateRot, (Satellite)1);

			satelliteWorlds[1] = sateTrans * sateRot * Worlds[i];
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &satelliteWorlds[1]);
			satelliteObjects[1]->DrawSubset(0);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &Worlds[i]);
			solarSystemObjects[i]->DrawSubset(0);
		}

		g_pd3dDevice->SetTransform(D3DTS_WORLD, &Worlds[i]);
		solarSystemObjects[i]->DrawSubset(0);


	}
}

FLOAT space_xPoint = 20.0f;
FLOAT space_yPoint = 0.0f;
FLOAT space_zPoint = 0.0f;
//우주선
VOID Spacecraft()
{
	//우주선 행렬
	D3DXMATRIXA16 spaceMat;

	D3DXMatrixIdentity(&spaceMat);

	//D3DXCreateBox(g_pd3dDevice, 3,3,3, &spacecraftObject, NULL);

	D3DXMatrixTranslation(&spaceMat, space_xPoint, space_yPoint, space_zPoint);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &spaceMat);
	spacecraftObject->DrawSubset(0);

}


VOID SetupMatrices()
{
	// Set up world matrix
	D3DXMATRIXA16 matWorld;
	D3DXMATRIXA16 sphereMat;

	//D3DXMatrixIdentity(&matWorld);
	//float연산을 좀 더 세밀하게 하기위해 1000으로나누어준다.

	//fAngle 매시간마다 반바퀴 회전하는각으로 설정. int로 변환해서 찍어보니 0~3나옴

	SolarSystem();
	Spacecraft();
	//D3DXMatrixRotationX(&sphereMat, fAngle);
	//matWorld = sphereMat;


	//g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	//기본값 0,70,-155
	D3DXVECTOR3 vEyePt(0.0f, 70.0f, -155.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);


	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 300.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}


VOID SetupLights()
{
	// Set up a material. The material here just has the diffuse and ambient
	// colors set to yellow. Note that only one material can be used at a time.

	//diffuse : 물체가 가지고있는 색상값
	//spercular : 
	//emit :  물체가 발산하는 발산광
	//Ambient : 주변환경광 
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));

	//랜덤으로 가져올 컬러값들 지정해두기.
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial(&mtrl);

	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));

	////light.Type = D3DLIGHT_POINT;
	//light.Type = D3DLIGHT_DIRECTIONAL;
	////이 시절엔 emmersivecolor세팅이 없었음..
	//light.Diffuse.r = 1.0f;
	//light.Diffuse.g = 1.0f;
	//light.Diffuse.b = 1.0f;

	//vecDir = D3DXVECTOR3(1.0f, 1.0f, 100.0f);
	//vecDir = D3DXVECTOR3( cosf( timeGetTime() / 350.0f ),   //라이트 방향 설정.
	//                      1.0f,
	//                      sinf( timeGetTime() / 350.0f ) );
	//light.Range = 1.0f;


	light.Type = D3DLIGHT_POINT;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;

	light.Range = 5000.0f;
	light.Position = D3DXVECTOR3(0.0f, 50.0f, 0.f);
	/*vecDir = D3DXVECTOR3(cosf(timeGetTime() / 350.0f),
		1.0f,
		sinf(timeGetTime() / 350.0f));*/
		//vecDir
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	g_pd3dDevice->SetLight(0, &light);
	g_pd3dDevice->LightEnable(0, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// Finally, turn on some ambient light.
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
	// Clear the backbuffer and the zbuffer
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// Begin the scene
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		// Setup the lights and materials
		SetupLights();

		// Setup the world, view, and projection matrices
		SetupMatrices();


		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x41:	//A
			space_xPoint -= 0.5f;
			return 0;
		case 0x44:	//D
			space_xPoint += 0.5f;
			return 0;
		case 0x53:	//S
			space_yPoint -= 0.5f;
			return 0;
		case 0x57:  //W
			space_yPoint += 0.5f;
			return 0;
		}
		return 0;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
	UNREFERENCED_PARAMETER(hInst);

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 04: Lights",
		WS_OVERLAPPEDWINDOW, 100, 100, 1200, 600,
		NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		// Create the geometry
		if (SUCCEEDED(InitGeometry()))
		{
			// Show the window
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			// Enter the message loop
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					Render();
			}
		}
	}

	UnregisterClass(L"D3D Tutorial", wc.hInstance);
	return 0;
}



