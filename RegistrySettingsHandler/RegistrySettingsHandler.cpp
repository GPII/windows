/*!
Registry Settings Handler

Copyright 2012 Astea Solutions AD

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/windows/LICENSE.txt
*/

#include <Windows.h>
#include <iostream>
#include <exception>
#include "json/json.h"
using namespace std;

HKEY hKeyFromString(string input)
{
	//TODO Cover all possible HKEY values
	if ("HKEY_LOCAL_MACHINE" == input)
		return HKEY_LOCAL_MACHINE;
	else if ("HKEY_CURRENT_USER" == input)
		return HKEY_CURRENT_USER;
	else 
		return NULL;
}

long getDwordValue (HKEY hKey, LPCWSTR path, LPCWSTR valueName)
{
	HKEY resKey;
	DWORD valueData;
	DWORD bufferSize(sizeof(DWORD));

	long codeOpen = RegOpenKeyExW(hKey, path, 0, KEY_QUERY_VALUE, &resKey);
	if (codeOpen != ERROR_SUCCESS)
	{
		throw 500;
	}

	long returnCode = RegQueryValueExW(resKey, valueName, NULL, NULL, (LPBYTE)&valueData, &bufferSize);
	RegCloseKey(hKey);
	if (returnCode == ERROR_SUCCESS)
	{
		return valueData;
	} else {
		throw 404;
	}
}

bool setDwordValue(HKEY hKey, LPCWSTR path, LPCWSTR valueName, DWORD valueData)
{
	HKEY resKey;
	RegCreateKeyExW(hKey, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &resKey, NULL);
	DWORD bufferSize(sizeof(DWORD));
    long returnCode = RegSetValueExW(resKey, valueName, 0, REG_DWORD, (const BYTE*)&valueData, bufferSize);
	RegCloseKey(hKey);
	// TODO Gracefully handle errors in setting a registry value.
    return (returnCode == ERROR_SUCCESS);
}

bool parseJson(const string jsonString, Json::Value &rootElement)
{
	Json::Reader reader;
	return reader.parse(jsonString, rootElement);
	// TODO Gracefully handle JSON parsing errors.
}

wstring toWideChar(const char * str)
{
	int length = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), 0, 0);
	LPWSTR wideString = SysAllocStringLen(0, length);
	MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), wideString, length);
	wstring result(wideString);
	SysFreeString(wideString);
	return result;
}

bool updateRegistry(Json::Value &jsonRoot, Json::Value &outputRoot)
{
	bool success = true;
	for (Json::ValueIterator iter = jsonRoot.begin(); iter != jsonRoot.end(); iter++) {
		Json::Value solutionsRoot = jsonRoot.get(iter.memberName(), Json::Value());
		Json::Value solutionsOutput;
		for(Json::ValueIterator itr = solutionsRoot["settings"].begin() ; itr != solutionsRoot["settings"].end() ; itr++ )
		{
			bool missingValue = false;
			Json::Value currentOption;
			const char * path = solutionsRoot["options"]["path"].asCString();
			const char * valueName = itr.memberName();
			long valueToSet = solutionsRoot["settings"][valueName].asUInt();
			
		    wstring updatedPath = toWideChar(path);
			wstring updatedValueName = toWideChar(valueName);
			long value = -1;
			try
			{
				value = getDwordValue(hKeyFromString(solutionsRoot["options"]["hKey"].asCString()), updatedPath.c_str(), updatedValueName.c_str());
			} catch (int exCode)
			{
				currentOption["statusCode"] = exCode;
				currentOption["oldValue"] = Json::Value();
				currentOption["newValue"] = Json::Value();
				success = false;
				missingValue = true;
			}

			bool valueSet = setDwordValue(hKeyFromString(solutionsRoot["options"]["hKey"].asCString()), updatedPath.c_str(), updatedValueName.c_str(), valueToSet);
			if (missingValue)
			{
				currentOption["oldValue"] = Json::Value();
			} else {
				currentOption["oldValue"] = value;
			}
			try
			{
				currentOption["newValue"] = getDwordValue(hKeyFromString(solutionsRoot["options"]["hKey"].asCString()), updatedPath.c_str(), updatedValueName.c_str());
			} catch (int exceptionCode) {
				// TODO Handle gracefully.
			}
			if (!valueSet)
			{
				success = false;
				currentOption["newValue"] = value;
			}
		
			solutionsOutput[valueName] = currentOption;
		}
		outputRoot[iter.memberName()]["results"] = solutionsOutput;
	}
	return success;
}

int main (int argc, char *argv[])
{
	// argv[1] - json model
	if (argc < 2)
	{
		cout << "Missing argument. Usage: RegistrySettingsHandler <JsonModel>.";
	}
	Json::Value root, optionsRoot, output;
	parseJson(argv[1], root);
	bool updateSuccessful = updateRegistry(root, output);
	cout << root;
	cout << output;
	system("Pause");
	return !updateSuccessful;
}