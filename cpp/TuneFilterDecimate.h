/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components TuneFilterDecimate.
 *
 * REDHAWK Basic Components TuneFilterDecimate is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components TuneFilterDecimate is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef TUNEFILTERDECIMATE_IMPL_H
#define TUNEFILTERDECIMATE_IMPL_H

#include "TuneFilterDecimate_base.h"
#include "DataTypes.h"
#include "Tuner.h"
#include "firfilter.h"
#include "FirFilterDesigner.h"
#include "Decimate.h"

class TuneFilterDecimate_i;

class TuneFilterDecimate_i : public TuneFilterDecimate_base
{
	ENABLE_LOGGING
public:
	TuneFilterDecimate_i(const char *uuid, const char *label);
	~TuneFilterDecimate_i();
	int serviceFunction();

	void start() throw (CORBA::SystemException, CF::Resource::StartError);

private:
	// Handle changes to the SRI
	void configureTFD(BULKIO::StreamSRI &sri);

	// ** Functions to generate tap coefficients for a lowpass filter.
	int generateTaps(const double& sFreq, const double& dOmega, const double& delta, Real fl = Real(0.5)); // returns # of taps
	void kaiser(RealArray &w, Real beta);
	Real in0(Real x);

	// Handle changes to tuner properties
	void configureFilter(const std::string& propid);
	void configureTuner(const std::string& propid);

	// Function to get an SRI keyword value
	template <typename TYPE> TYPE getKeywordByID(BULKIO::StreamSRI &sri, CORBA::String_member id, bool &valid) {
		/****************************************************************************************************
		 * Description: Retrieve the value assigned to a given id.
		 * sri   - StreamSRI object to process
		 * id    - Keyword identifier string
		 * valid - Flag to indicate whether the returned value is valid (false if the keyword doesn't exist)
		 ****************************************************************************************************/
		valid = false;
		TYPE value;

		for(unsigned int i=0; i < sri.keywords.length(); i++) {
			if(!strcmp(sri.keywords[i].id, id)) {
				valid = true;
				if (sri.keywords[i].value >>= value)
					break;
				//try with double and float to extract it and see if we can make it happen if the
				//format of this keyword is different than we expect
				double d;
				if (sri.keywords[i].value >>= d)
				{
					value=static_cast<TYPE>(d);
					break;
				}
				float f;
				if (sri.keywords[i].value >>= f)
				{
					value=static_cast<TYPE>(f);
					break;
				}
				valid = false;
			}
		}
		return value;
	}

	// Function to set an SRI keyword value
	template <typename TYPE> bool setKeywordByID(BULKIO::StreamSRI &sri, CORBA::String_member id, TYPE value) {
		/****************************************************************************************************************
		 * Description: Set a value to an existing keyword, or add a new keyword/value pair if it doesn't already exist.
		 * sri   - StreamSRI object to process
		 * id    - Keyword identifier string
		 * value - The value to set.
		 */
		CORBA::Any corbaValue;
		corbaValue <<= value;
		unsigned long keySize = sri.keywords.length();

		// If keyword is found, set it and return true
		for(unsigned long i=0; i < keySize; i++) {
			if(!strcmp(sri.keywords[i].id, id)) {
				sri.keywords[i].value = corbaValue;
				return true;
			}
		}

		// Otherwise, add keyword and set it before returning true
		sri.keywords.length(keySize+1);
		if(sri.keywords.length() != keySize+1) // Ensure the length has been adjusted
		{
			std::cout<<"RIGHT HERE --- "<<std::endl;
			return false;
		}
		sri.keywords[keySize].id = CORBA::string_dup(id);
		sri.keywords[keySize].value = corbaValue;
		return true;
	}

	// Processing classes
	Tuner *tuner;
	firfilter *filter;
	Decimate *decimate;

	// Internal buffers
	ComplexVector tunerInput;
	ComplexVector decimateOutput;
	std::vector<float> floatBuffer; // output buffer

	// Input buffers for the FIR filter are fed as output buffers to the Tuner object.
	// Output buffers for the FIR filter are set as input buffers to the Decimate object.
	firfilter::complexVector f_complexIn;
	firfilter::realVector f_realOut;
	firfilter::complexVector f_complexOut;
	// All of these are REQUIRED by firfilter's constructor, whether we are filtering real or complex data.
	// DO NOT REMOVE.

	RealFFTWVector filterCoeff; // To set the taps for the filter. Only real taps for current implementation.

	// Private variables
	Real inputSampleRate;
	Real outputSampleRate;
	double chan_if;
	bool tuningRFChanged; // Used to indicate if TuningRF has been changed so the CHAN_RF keyword can be added to SRI
	bool RemakeFilter;    // Used to indicate we must redo the filter
	std::string streamID;
	bool inputComplex;
	//values set in TuneFilterDecimate.cpp
	const static size_t MIN_NUM_TAPS;
	const static size_t MAX_NUM_TAPS;
	const static size_t MIN_FFT_SIZE;
	const static size_t MAX_FFT_SIZE;
    FirFilterDesigner filterdesigner_;

    // Property Change Listener Callbacks
    void TuningNormChanged(const double *oldValue, const double *newValue);
    void TuningIFChanged(const double *oldValue, const double *newValue);
    void TuningRFChanged(const unsigned long long *oldValue, const unsigned long long *newValue);
    void FilterBWChanged(const float *oldValue, const float *newValue);
    void DesiredOutputRateChanged(const float *oldValue, const float *newValue);
    void filterPropsChanged(const filterProps_struct *oldValue, const filterProps_struct *newValue);

    boost::mutex TuneFilterDecimateLock_;
};

#endif
