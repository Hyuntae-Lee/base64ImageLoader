// base64ImageLoaderMFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "base64ImageLoaderMFC.h"
#include <vector>
#include <fstream>
#include <tuple>
#include <gdiplus.h>
#include <cpprest\json.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
using namespace Gdiplus;

long getFileSize(wstring filePath);
bool parseAnalyzeResult(
	std::vector<tuple<std::wstring, std::vector<unsigned char>, pair<double, double>>>& out_list,
	std::wstring resutlFilePaht);

static const wstring kStrFilePath = L"D:/HCT_DATA/20180515/000001/FDS_180515_143120_OS/vuno_analysis_result.json";

int main()
{
    HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr) {
		return 1;
	}

	if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0)) {
		return 1;
	}

    // TODO: change error code to suit your needs
	vector<tuple<wstring, std::vector<unsigned char>, pair<double, double>>> dataList;

	parseAnalyzeResult(dataList, kStrFilePath);

	auto data = get<1>(dataList[0]);

	// make memory stream
	const size_t kBufferSize = data.size() * sizeof(unsigned char);

	HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, kBufferSize);
	if (!hMem) {
		AfxThrowMemoryException();
		return 1;
	}
		
	LPVOID pImageBuffer = ::GlobalLock(hMem);

	CopyMemory(pImageBuffer, &data[0], kBufferSize);
	
	::GlobalUnlock(hMem);

	CComPtr<IStream> spStream;
	if (::CreateStreamOnHGlobal(hMem, FALSE, &spStream) != S_OK) {
		return 1;
	}

	auto pImage = Image::FromStream(spStream);

	//pImage->Save(_T("D:/TTT/sampleImage.png"), nullptr);


    return 0;
}

bool parseAnalyzeResult(
	std::vector<tuple<std::wstring, std::vector<unsigned char>, pair<double, double>>>& out_list,
	std::wstring resutlFilePaht)
{
	// read from file
	auto fileSize = getFileSize(resutlFilePaht);
	// - open
	std::wifstream wif(resutlFilePaht, std::ios::in | std::wifstream::binary);
	if (!wif.is_open()) {
		return false;
	}
	// - read and parse
	web::json::value jsonValue;
	{
		wchar_t* pContentBuffer = new wchar_t[fileSize + 1];
		memset(pContentBuffer, 0, sizeof(wchar_t) * (fileSize + 1));

		wif.read(pContentBuffer, fileSize);
		// - close
		wif.close();

		// parse
		std::error_code err;
		jsonValue = web::json::value::parse(pContentBuffer, err);
		if (err.value() != 0) {
			return false;
		}

		delete pContentBuffer;
	}

	// get data
	web::json::array findings = jsonValue[L"findings"].as_array();
	for (auto item : findings) {
		int is_abnormal = item[L"is_abnormal"].as_integer();
		if (is_abnormal == 0) {
			continue;
		}

		std::wstring strName = item[L"name"].as_string();
		std::wstring roiDataBase64 = item[L"b64encoded_roi_image"].as_string();

		web::json::array label_positions = item[L"label_positions"].as_array();
		auto firstPosition = label_positions[0];
		double x = firstPosition[L"x"].as_double();
		double y = firstPosition[L"y"].as_double();

		auto roiData = utility::conversions::from_base64(roiDataBase64);

		out_list.push_back(make_tuple(strName, roiData, make_pair(x, y)));
	}
}

long getFileSize(wstring filePath)
{
	streampos fsize = 0;
	wifstream file(filePath, ios::binary);

	fsize = file.tellg();
	file.seekg(0, ios::end);
	fsize = file.tellg() - fsize;
	file.close();

	return fsize;
}

string convertToString(std::wstring string)
{
	std::string ret;

	ret.assign(string.begin(), string.end());

	return ret;
}
