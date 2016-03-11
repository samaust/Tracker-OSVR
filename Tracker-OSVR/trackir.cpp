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

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "com_samaust_tracker_osvr_json.h"

// Standard includes
#include <iostream>
#include <cmath>

#include "trackir.h"

trackir::trackir(OSVR_PluginRegContext ctx)
{
	/// Create the initialization options
	OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

	// configure our tracker
	osvrDeviceTrackerConfigure(opts, &m_tracker);

	/// Create the device token with the options
	m_dev.initAsync(ctx, "Tracker", opts);

	/// Send JSON descriptor
	m_dev.sendJsonDescriptor(com_samaust_tracker_osvr_json);

	/// Register update callback
	m_dev.registerUpdateCallback(this);

	initialized = false;

/*	pCameraCollection = new INPCameraCollection;
	pCamera = new INPCamera;
	pFrame = new INPCameraFrame;
	pVector = new INPVector;
*/
	zero_vx.dblVal = 0.0f;
	zero_vy.dblVal = 0.0f;
	zero_vz.dblVal = 0.0f;
	zero_vyaw.dblVal = 0.0f;
	zero_vpitch.dblVal = 0.0f;
	zero_vroll.dblVal = 0.0f;

	vx.dblVal = 0.0f;
	vy.dblVal = 0.0f;
	vz.dblVal = 0.0f;
	vyaw.dblVal = 0.0f;
	vpitch.dblVal = 0.0f;
	vroll.dblVal = 0.0f;

	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_yaw = 0;
	m_pitch = 0;
	m_roll = 0;
}

trackir::~trackir(void)
{
/*	if (pCameraCollection)
		delete pCameraCollection;
	if (pCamera)
		delete pCamera;
	if (pFrame)
		delete pFrame;
	if (pVector)
		delete = pVector;
*/
}

HRESULT trackir::initTrackir(float X, float Y, float Z, float Yaw, float Pitch, float Roll)
{
	// TODO : deallocate stuff if the functions fails
	LONG count = 0;

	// create CameraCollection and Vector objects:

	HRESULT hr = CoCreateInstance(__uuidof(NPCameraCollection), NULL, CLSCTX_ALL,
						  __uuidof(INPCameraCollection), (void **) &pCameraCollection);
	if (FAILED(hr))
		return -1;

	hr = CoCreateInstance(__uuidof(NPVector), NULL, CLSCTX_ALL,
							__uuidof(INPVector), (void **) &pVector);
	if (FAILED(hr))
		return -1;

	// find all the cameras:
	hr = pCameraCollection->Enum();
	if (FAILED(hr))
		return -1;

	hr = pCameraCollection->get_Count(&count);
	if (FAILED(hr))
		return -1;

	if (count < 1) 
		return -1;

	// get the first camera
	hr = pCameraCollection->Item(0, &pCamera);
	if (FAILED(hr))
		return -1;

	// turn on illumination LEDs
	hr = pCamera->SetLED(NP_LED_ONE, VARIANT_TRUE);
	if (FAILED(hr))
		return -1;
	
	hr = pCamera->SetLED(NP_LED_FOUR, VARIANT_TRUE);
	if (FAILED(hr))
		return -1;

//	VARIANT var;
//	var.boolVal = VARIANT_TRUE;
//	hr = pCamera->SetOption(NP_OPTION_SEND_EMPTY_FRAMES, var);
//	if (FAILED(hr))
//		return -1;

	hr = pCamera->Open();
	if (FAILED(hr))
		return -1;

	hr = pCamera->Start();
	if (FAILED(hr))
		return -1;

	hr = pVector->Reset();
	if (FAILED(hr))
		return -1;

	initialized = true;

	//trackir::resetVectorsZeros();

	m_XScale = X;
	m_YScale = Y;
	m_ZScale = Z;
	m_YawScale = Yaw;
	m_PitchScale = Pitch;
	m_RollScale = Roll;

	return 0;
}

void trackir::uninitTrackir(void) // shut everything down
{
	if (initialized)
	{
		pCamera->SetLED(NP_LED_ONE, VARIANT_FALSE);
		pCamera->SetLED(NP_LED_FOUR, VARIANT_FALSE);
		pCamera->Stop();
		pCamera->Close();
		pCamera->Release(); // added
	
		pVector->Release();

		pCameraCollection->Release(); // added

		initialized = false;
	}
}

HRESULT trackir::resetVectorsZeros(void)
{
	if (initialized)
	{
		VARIANT vx, vy, vz;
		VARIANT vyaw, vpitch, vroll;

		pFrame = NULL; // may not be necessary
		HRESULT hr = pCamera->GetFrame(0, &pFrame);
		if (FAILED(hr))
			return -1;

		while (pFrame == NULL)
		{
			pFrame = NULL; // Without that, sometimes it crashes
			pCamera->GetFrame(0, &pFrame);
		}

		if(pFrame!=0) // added
		{
			hr = pVector->Update(pCamera, pFrame);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_X(&vx);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_Y(&vy);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_Z(&vz);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_Yaw(&vyaw);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_Pitch(&vpitch);
			if (FAILED(hr))
				return -1;

			hr = pVector->get_Roll(&vroll);
			if (FAILED(hr))
				return -1;

			// Initialise the center position of the vector
			zero_vx.dblVal = vx.dblVal;
			zero_vy.dblVal = vy.dblVal;
			zero_vz.dblVal = vz.dblVal;
			zero_vyaw.dblVal = vyaw.dblVal;
			zero_vpitch.dblVal = vpitch.dblVal;
			zero_vroll.dblVal = vroll.dblVal;

			hr = pFrame->Free();
			if (FAILED(hr))
				return -1;

			pFrame->Release();
		}
	}
	else
		trackir::initTrackir();

	return 0;
}

OSVR_ReturnCode trackir::update() {
	if (initialized)
	{
		pFrame = NULL;
		HRESULT hr = pCamera->GetFrame(0, &pFrame);

		if (pFrame != 0)
		{
			hr = pVector->Update(pCamera, pFrame);
			//		if (FAILED(pVector->Update(pCamera, pFrame)))
			//			return -1;

			hr = pVector->get_X(&vx);
			//		if (FAILED(hr))
			//			return -1;

			hr = pVector->get_Y(&vy);
			//		if (FAILED(hr))
			//			return -1;

			hr = pVector->get_Z(&vz);
			//		if (FAILED(hr))
			//			return -1;

			hr = pVector->get_Yaw(&vyaw);
			//		if (FAILED(hr))
			//			return -1;

			hr = pVector->get_Pitch(&vpitch);
			//		if (FAILED(hr))
			//			return -1;

			hr = pVector->get_Roll(&vroll);
			//		if (FAILED(hr))
			//			return -1;

			m_x = (float)(vx.dblVal - zero_vx.dblVal)*m_XScale;
			m_y = (float)(vy.dblVal - zero_vy.dblVal)*m_YScale;
			m_z = (float)(vz.dblVal - zero_vz.dblVal)*m_ZScale;
			m_yaw = (float)(vyaw.dblVal - zero_vyaw.dblVal)*m_YawScale;
			m_pitch = (float)(vpitch.dblVal - zero_vpitch.dblVal)*m_PitchScale;
			m_roll = (float)(vroll.dblVal - zero_vroll.dblVal)*m_RollScale;

			pFrame->Release();
		}
		else
		{
			// Camera out of sight, return centered position
			m_x = 0.0f;
			m_y = 0.0f;
			m_z = 0.0f;
			m_yaw = 0.0f;
			m_pitch = 0.0f;
			m_roll = 0.0f;
		}
	}
	else
		trackir::initTrackir();

	// Euler to quaternion
	// Source : http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
	
	// Assuming the angles are in radians.
	double c1 = cos(m_yaw / 2);
	double s1 = sin(m_yaw / 2);
	double c2 = cos(m_pitch / 2);
	double s2 = sin(m_pitch / 2);
	double c3 = cos(m_roll / 2);
	double s3 = sin(m_roll / 2);
	double c1c2 = c1*c2;
	double s1s2 = s1*s2;

	// Report pose
	OSVR_Pose3 pose;
	pose.translation.data[0] = m_x;
	pose.translation.data[1] = m_y;
	pose.translation.data[2] = m_z;
	pose.rotation.data[0] = c1c2*c3 - s1s2*s3;;
	pose.rotation.data[1] = c1c2*s3 + s1s2*c3;
	pose.rotation.data[2] = s1*c2*c3 + c1*s2*s3;
	pose.rotation.data[3] = c1*s2*c3 - s1*c2*s3;
	osvrDeviceTrackerSendPose(m_dev, m_tracker, &pose, 0);

	return OSVR_RETURN_SUCCESS;
}

void trackir::flushFrames(void)
{
	if (initialized)
	{
		pFrame = NULL; // Without that, sometimes it crashes
		pCamera->GetFrame(0, &pFrame);

		if(pFrame!=NULL)
		{
			while(pFrame!=NULL)
			{
				pFrame->Release();
				pFrame = NULL; // Without that, sometimes it crashes
				pCamera->GetFrame(0, &pFrame);
			}
		}
	}
	else trackir::initTrackir();
}
