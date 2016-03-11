// Copyright 2016 Samuel Austin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "optitrack.h"

class trackir
{
	public:
		trackir(OSVR_PluginRegContext ctx);
		~trackir();

		HRESULT initTrackir(float = 1.0f, float = 1.0f, float = 1.0f, float = 1.0f, float = 1.0f, float = 1.0f);
		void uninitTrackir();
		HRESULT resetVectorsZeros();
		OSVR_ReturnCode update();
		void flushFrames();

		INPCameraCollection *pCameraCollection;
		INPCamera *pCamera;
		INPCameraFrame *pFrame;
		INPVector *pVector;

	private:
		osvr::pluginkit::DeviceToken m_dev;
		OSVR_TrackerDeviceInterface m_tracker;

		bool initialized;

		// Center offsets
		VARIANT zero_vx, zero_vy, zero_vz;
		VARIANT zero_vyaw, zero_vpitch, zero_vroll;

		// Raw measurements
		VARIANT vx, vy, vz;
		VARIANT vyaw, vpitch, vroll;

		// Scaling
		float m_XScale, m_YScale, m_ZScale;
		float m_YawScale, m_PitchScale, m_RollScale;

		// Position and orientation returned to game
		float m_x, m_y, m_z;
		float m_yaw, m_pitch, m_roll;
};