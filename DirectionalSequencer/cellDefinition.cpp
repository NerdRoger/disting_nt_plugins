#include "cellDefinition.h"


const char* const CellNames[] = {
	"Direction",
	"Value",
	"Velocity",
	"Probability",
	"Ratchets",
	"Rest After",
	"Gate Len",
	"Drift Prob",
	"Max Drift",
	"Repeats",
	"Glide",
	"Accum Add",
	"Accum Times",
};


const CellDefinition CellDefinitions[] = {
	// Don't Change
	//   vvvv
	{ "Direction",   CellNames[0],   0, 8,   0,   0, kNT_unitNone,    kNT_scalingNone, "  Direction of next step. Blank = keep going" },
	{ "Value",       CellNames[1],   0, 10,  5,   3, kNT_unitVolts,   kNT_scaling1000, "  Step Value, 0-10V. Off/Atten in parameters" },
	{ "Velocity",    CellNames[2],   1, 127, 64,  0, kNT_unitNone,    kNT_scalingNone, "             Step velocity, 1-127" },
	{ "Probability", CellNames[3],   0, 100, 100, 0, kNT_unitPercent, kNT_scalingNone, "            Step probability, 0-100%" },
	{ "Ratchets",    CellNames[4],   0, 7,   0,   0, kNT_unitNone,    kNT_scalingNone, "   Number of ratchets. Needs a steady clock" },
	{ "RestAfter",   CellNames[5],   0, 7,   0,   0, kNT_unitNone,    kNT_scalingNone, "  Play step N times, then rest. 0 = no rests" },
	{ "GateLen",     CellNames[6],   0, 100, 75,  0, kNT_unitPercent, kNT_scalingNone, " Gate len, 0-100% of clock or max gate length" },
	{ "DriftProb",   CellNames[7],   0, 100, 0,   0, kNT_unitPercent, kNT_scalingNone, "    Probability the value will drift, 0-100%" },
	{ "MaxDrift",    CellNames[8],   0, 10,  0,   3, kNT_unitVolts,   kNT_scaling1000, "   Max amount the value will drift by, 0-10V" },
	{ "Repeats",     CellNames[9],   0, 7,   0,   0, kNT_unitNone,    kNT_scalingNone, "        Repeat the step this many times" },
	{ "Glide",       CellNames[10],  0, 100, 0,   0, kNT_unitPercent, kNT_scalingNone, "    How quickly to glide, 0-100% of gate len" },
	{ "AccumAdd",    CellNames[11], -1, 1,   0,   3, kNT_unitVolts,   kNT_scaling1000, " How much gets added to this cell each visit" },
	{ "AccumTimes",  CellNames[12],  0, 7,   0,   0, kNT_unitNone,    kNT_scalingNone, "  Clear accumulator for cell after N visits" },
};
