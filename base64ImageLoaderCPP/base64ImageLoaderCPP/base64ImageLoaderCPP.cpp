// base64ImageLoaderCPP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <tuple>
#include <fstream>
#include <cpprest\json.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>

using namespace std;

long getFileSize(wstring filePath);
bool parseAnalyzeResult(
	std::vector<tuple<std::wstring, std::vector<unsigned char>, pair<double, double>>>& out_list,
	std::wstring resutlFilePaht);

static const wstring kStrFilePath = L"D:/HCT_DATA/20180515/000001/FDS_180515_143120_OS/vuno_analysis_result.json";

int main()
{
	vector<tuple<wstring, std::vector<unsigned char>, pair<double, double>>> dataList;

	parseAnalyzeResult(dataList, kStrFilePath);

	auto data = get<1>(dataList[0]);

	Gdiplus::Image::From

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