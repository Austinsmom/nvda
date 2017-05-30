/*
Code for C dll bridge to UWP OCR.
This file is a part of the NVDA project.
URL: http://www.nvaccess.org/
Copyright 2017 NV Access Limited.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0, as published by
    the Free Software Foundation.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
This license can be found at:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>
#include <windows.h>
#include <cstring>
#include <common/log.h>
#include "utils.h"
#include "uwpOcr.h"

using namespace std;
using namespace Platform;
using namespace concurrency;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Windows::Media::Ocr;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization;
using namespace Windows::Graphics::Imaging;

UwpOcr* __stdcall uwpOcr_initialize(const char16* language) {
	auto engine = OcrEngine::TryCreateFromLanguage(ref new Language(ref new String(language)));
	if (!engine)
		return NULL;
	auto instance = new UwpOcr;
	instance->engine = engine;
	return instance;
}

void __stdcall uwpOcr_terminate(UwpOcr* instance) {
	delete instance;
}

void __stdcall uwpOcr_recognize(UwpOcr* instance, const RGBQUAD* image, unsigned int width, unsigned int height) {
	SoftwareBitmap^ sbmp;
	try {
	unsigned int numBytes = sizeof(RGBQUAD) * width * height;
	auto buf = ref new Buffer(numBytes);
	buf->Length = numBytes;
	BYTE* bytes = getBytes(buf);
	memcpy(bytes, image, numBytes);
	LOG_INFO(L"before sbmp");
	sbmp = SoftwareBitmap::CreateCopyFromBuffer(buf, BitmapPixelFormat::Bgra8, width, height);
	} catch (Platform::Exception^ e) {
		LOG_ERROR(L"Error " << e->HResult << L": " << e->Message->Data());
		return;
	}
	task<OcrResult^> ocrTask = create_task(instance->engine->RecognizeAsync(sbmp));
	ocrTask.then([instance] (OcrResult^ result) {
		LOG_ERROR(result->Text->Data());
	}).then([instance] (task<void> previous) {
		// Catch any unhandled exceptions that occurred during these tasks.
		try {
			previous.get();
		} catch (Platform::Exception^ e) {
			LOG_ERROR(L"Error " << e->HResult << L": " << e->Message->Data());
			//instance->callback(NULL);
		}
	});
}
