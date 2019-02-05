#pragma once

//-----CUT PARAMETERS-----//
#define TIME_CUT_LOW -10//-15
#define TIME_CUT_HIGH 10//-10

#define CHI2_NDF_CUT 10.0f


#define NUM_BOARDS 16
#define NUM_CHANS 64
#define BOARD_START_ID 100

//-----TEMPORARY COORDINATES-----//
#define CHAN_MID_LOW 31
#define CATHODE_WIDTH 12.954f //0.510"
#define CATHODE_CENTER CATHODE_WIDTH/2.
#define CATHODE_GAP 2.032f //0.08"
#define CATHODE_MID_GAP CATHODE_GAP/2.

//-----BEGIN REAL COORDINATES-----//
#define CATHODE_WIDTH_X 14.5f //0.569"
#define CATHODE_PITCH_X 15.5f //0.609"
#define CATHODE_WIDTH_Y 13.0f //0.510"
#define CATHODE_PITCH_Y 15.0f//0.590"
#define PANEL_THICKNESS 20.0f
#define GAS_GAP_THICKNESS 10.0f
#define WIRE_PANEL_GAP 5.0f



#define SPACING 513.0f
#define SPACING2 270.0f

#define Z_UNCERTAINTY 2.0f
