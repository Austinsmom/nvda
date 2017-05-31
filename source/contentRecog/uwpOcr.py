#contentRecog/uwpOcr.py
#A part of NonVisual Desktop Access (NVDA)
#Copyright (C) 2017 NV Access Limited
#This file is covered by the GNU General Public License.
#See the file COPYING for more details.

"""Recognition of text using the UWP OCR engine included in Windows 10.
"""

import ctypes
import json
from . import ContentRecognizer, LinesWordsResult

uwpOcr_Callback = ctypes.CFUNCTYPE(None, ctypes.c_wchar_p)
DLL_FILE = ur"lib\nvdaHelperLocalWin10.dll"

class UwpOcr(ContentRecognizer):

	def __init__(self):
		self._dll = ctypes.windll[DLL_FILE]

	def recognize(self, pixels, left, top, width, height, onResult):
		self._onResult = onResult
		@uwpOcr_Callback
		def callback(result):
			# If self._onResult is None, recognition was cancelled.
			if self._onResult:
				if result:
					data = json.loads(result)
					self._onResult(LinesWordsResult(data, left, top))
				else:
					self._onResult(RuntimeError("UWP OCR failed"))
			self._dll.uwpOcr_terminate(self._handle)
			self._callback = None
			self._handle = None
		self._callback = callback
		self._handle = self._dll.uwpOcr_initialize(u"en", callback)
		self._dll.uwpOcr_recognize(self._handle, pixels, width, height)

	def cancel(self):
		self._onResult = None
