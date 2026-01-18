#include "common.h"
#include "cellDefinition.h"


// orders must match the CellDataType enum
HIDDEN const CellDefinition CellDefinition::All[static_cast<uint16_t>(CellDataType::NumCellDataTypes)] = {
	// Don't Change
	//   vvvv
	{ "Direction",   "Direction",    0,    8,     0,    kNT_unitNone,    kNT_scalingNone, 10, "Direction of next step. Blank = keep going" },
	{ "Value",       "Value",        0,    10000, 5000, kNT_unitVolts,   kNT_scaling1000, 10, "Step Value, 0-10V. Off/Atten in parameters" },
	{ "Velocity",    "Velocity",     1,    127,   64,   kNT_unitNone,    kNT_scalingNone, 65, "Step velocity, 1-127" },
	{ "Glide",       "Glide",        0,    100,   0,    kNT_unitPercent, kNT_scalingNone, 20, "How quickly to glide, 0-100% of gate len" },
	{ "GateLen",     "Gate Len",     0,    100,   75,   kNT_unitPercent, kNT_scalingNone,  5, "Gate len, 0-100% of clock or max gate length" },
	{ "Ratchets",    "Ratchets",     0,    7,     0,    kNT_unitNone,    kNT_scalingNone, 15, "Number of ratchets. Needs a steady clock" },
	{ "Probability", "Probability",  0,    100,   100,  kNT_unitPercent, kNT_scalingNone, 60, "Step probability, 0-100%" },
	{ "RestAfter",   "Rest After",   0,    7,     0,    kNT_unitNone,    kNT_scalingNone, 10, "Play step N times, then rest. 0 = no rests" },
	{ "Repeats",     "Repeats",      0,    7,     0,    kNT_unitNone,    kNT_scalingNone, 40, "Repeat the step this many times" },
	{ "DriftProb",   "Drift Prob",   0,    100,   0,    kNT_unitPercent, kNT_scalingNone, 20, "Probability the value will drift, 0-100%" },
	{ "MaxDrift",    "Max Drift",    0,    10000, 0,    kNT_unitVolts,   kNT_scaling1000, 15, "Max amount the value will drift by, 0-10V" },
	{ "AccumAdd",    "Accum Add",   -1000, 1000,  0,    kNT_unitVolts,   kNT_scaling1000,  5, "How much gets added to this cell each visit" },
	{ "AccumTimes",  "Accum Times",  0,    7,     0,    kNT_unitNone,    kNT_scalingNone, 10, "Clear accumulator for cell after N visits" },
	{ "TieSteps",    "Tie Steps",    0,    7,     0,    kNT_unitNone,    kNT_scalingNone, 30, "Tie this many steps after this cell" },
};
