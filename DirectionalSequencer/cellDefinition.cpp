#include "cellDefinition.h"


const CellDefinition CellDefinition::All[static_cast<uint16_t>(CellDataType::NumCellDataTypes)] = {
	// Don't Change
	//   vvvv
	{ "Direction",   "Direction",    0, 8,   0,   kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    10, "Direction of next step. Blank = keep going" },
	{ "Value",       "Value",        0, 10,  5,   kNT_unitVolts,   kNT_scaling1000, 1000, 0.0005f, 10, "Step Value, 0-10V. Off/Atten in parameters" },
	{ "Velocity",    "Velocity",     1, 127, 64,  kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    65, "Step velocity, 1-127" },
	{ "Probability", "Probability",  0, 100, 100, kNT_unitPercent, kNT_scalingNone, 1,    0.5f,    60, "Step probability, 0-100%" },
	{ "Ratchets",    "Ratchets",     0, 7,   0,   kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    15, "Number of ratchets. Needs a steady clock" },
	{ "RestAfter",   "Rest After",   0, 7,   0,   kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    10, "Play step N times, then rest. 0 = no rests" },
	{ "GateLen",     "Gate Len",     0, 100, 75,  kNT_unitPercent, kNT_scalingNone, 1,    0.5f,     5, "Gate len, 0-100% of clock or max gate length" },
	{ "DriftProb",   "Drift Prob",   0, 100, 0,   kNT_unitPercent, kNT_scalingNone, 1,    0.5f,    20, "Probability the value will drift, 0-100%" },
	{ "MaxDrift",    "Max Drift",    0, 10,  0,   kNT_unitVolts,   kNT_scaling1000, 1000, 0.0005f, 15, "Max amount the value will drift by, 0-10V" },
	{ "Repeats",     "Repeats",      0, 7,   0,   kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    40, "Repeat the step this many times" },
	{ "Glide",       "Glide",        0, 100, 0,   kNT_unitPercent, kNT_scalingNone, 1,    0.5f,    20, "How quickly to glide, 0-100% of gate len" },
	{ "AccumAdd",    "Accum Add",   -1, 1,   0,   kNT_unitVolts,   kNT_scaling1000, 1000, 0.0005f,  5, "How much gets added to this cell each visit" },
	{ "AccumTimes",  "Accum Times",  0, 7,   0,   kNT_unitNone,    kNT_scalingNone, 1,    0.5f,    10, "Clear accumulator for cell after N visits" },
};
