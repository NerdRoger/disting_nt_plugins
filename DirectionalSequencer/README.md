# Directional Sequencer

Directional Sequencer is a 2‑D grid-based step sequencer plugin for the disting NT, where each step can be set to move to any of its neighboring steps in an 8x4 grid. It provides multiple independent playheads that move across the grid, and each cell contains various attributes for that step such as value (pitch), probability, ratchets, and others as defined below.

## Overview
When you first add the Directional Sequencer, you can specify how many playheads you would like to use, up to 8.  Choose just a single playhead to get started, then you can respecify to add more when you're ready.  More details below in the Playheads section.  When you first start, the sequencer will look like this:

![alt text](<../images/Image 101.png>)

### User Interface and Controls

As you can see in the image above, the UI is divided into 4 sections:
- On the left, the playhead list.  We just have a single one here.
- In the middle, the grid field.  These are your steps.  Here, you can see that the top-left cell is selected.
- On the right, the per-step attributes of the selected cell.  The UI can only show 5 attributes at a time, but the list scrolls, and there are actually 15 different attributes per cell you can select.
- On the bottom, you can see a help section showing you the controls.  As you scroll through the attributes, this area will change to show you what each of them means.

<br>

Similar to other areas of the disting NT, each control has reminder text above it reminding you of the controls.  They are:

| Control | Effect |
|--------|--------|
| Left Encoder Turn | Change the selected cell in the X direction. |
| Right Encoder Turn | Change the selected cell in the Y direction. |
| Left Pot Turn | Select the playhead. Currently selected head is identified by the play icon next to it. |
| Center Pot Turn | Scroll through the list of cell attributes. Above, you can see the "Direction" attribute is selected. |
| Right Pot Turn | Change the value of the selected attribute for the selected cell. |
| Right Encoder Quick Press | Lock the UI to prevent accidental changes. |
| Right Encoder Long Press | Set the starting step of the selected playhead. Identified by the marquee border around the cell. |
| Right Pot Quick Press | Set the value of the selected attribute to its default value for the selected cell. |
| Right Pot Long Press | Set the value of the selected attribute to its default value for ALL cells. |

<br>

So using this knowledge of the controls, in the image below, I have navigated down to the Gate Length attribute, then moved thru each cell, assigning a value.  You can see that each cell in the grid shows you a visual representation of the attribute value.  This visual representation will cary from one attribute to the next, but should always give you an idea of all of the values for that attribute at a glance.  You can also see I have moved the selection to the last cell in the third row, and the Gate Length value for that cell is set to 62.  The starting cell for this playhead was changed to be the fourth cell on the first row, as shown by the marquee.  Finally the currently playing step is the 7th step in the first row.

<br>

![alt text](<../images/Image 102.png>)

## Cell Attributes
Each cell carries the following attributes:

| Attribute | Description |
|--------|--------|
| `Direction` | The direction to move for the next step. If empty, play continues in the current direction. If a playhead’s initial cell has no value, it moves right (east). Directions that would move outside the grid wrap to the opposite side (Pac‑Man warp). |
| `Value` | The step value (typically pitch), modular; range 0–10V. |
| `Velocity` | The step velocity. Can be sent to a playhead output and used to create “velocity gates”; range 1–127. |
| `Glide` | How quickly the value glides to the next step, expressed as a percentage of the gate length. 0% = no glide; 100% = glide over the entire step. |
| `Gate Length` | Gate duration for the step as a percentage of either the clock period or the defined max gate length. 0% = no gate; 100% = full legato. |
| `Ratchets` | Number of evenly‑divided ratchets (based on incoming clock) to play for this step, up to 8. Irregular clocks disable ratcheting.  Steps that are part of a tie also cannot ratchet. |
| `Tie Steps` | The number of steps following this step that will be tied to it, meaning the gate will be held high for the duration of all tied steps, and the value and velocity for this step will be used on all tied steps.  When a tie is playing, the current step indicator changes to be thicker.  This allows the user to understand that the tie might be currently overriding other attributes. |
| `Probability` | Percent chance that the step will emit a gate. |
| `Rest After` | The step will play this many times, then rest on the next visit, then repeat the cycle (e.g., 3 → play 3 times, rest once). |
| `Repeats` | If set, the step repeats this many times before advancing. |
| `Drift Probability` | Percent chance the cell’s value will “drift” (randomly vary) on a visit. |
| `Max Drift` | Maximum amount the value can drift away from the value (randomly up or down) when drift occurs. |
| `Accumulator Add` | Amount to add (or subtract) to the cell value each visit; accumulated across visits to change pitch progressively. Range −5V to +5V. |
| `Accumulator Times` | Number of visits the accumulation is applied before the accumulation process starts over. |
| `Mute` | If set, the step will not emit a gate, effectively muting the step. |

## Playheads
You can have up to 8 playheads each moving through the grid.  When you have multiple playheads, you can select them by turning the left pot.  The cell attributes remain the same across all playheads, but there are a number of parameters designed to make each playhead unique!  Each playhead can have its own starting position (chosen using Right Encoder, as above), so that each one can start in different areas of the grid.

Additionally, each playhead has its own routing and option parameters:

### Playhead Routing

| Parameter | Description |
|--------|--------|
| `Clock` | Required. The disting NT input/bus to advance the playhead. |
| `Clock Divisor` | Only advance the playhead after this many clock triggers. Handy to have multiple playheads running at different rates, all from the same clock. |
| `Clock Offset` | A phase offset applied when the clock is divided. For instance, if your divisor is set to 4, and you set the offset to 2, you are advancing this playhead on the off beats. |
| `Reset` | The disting NT input/bus to send a trigger to in order to reset the playhead to its initial position. |
| `Value` | The disting NT output/bus where the value (pitch) for the current step of this playhead is sent. |
| `Gate` | The disting NT output/bus where the gate for the current step of this playhead is sent. |
| `Velocity Gate Min` | The minimum amplitude for the gate. The step's velocity is applied on top of this, creating a "velocity gate", to a maximum of 5V.  Setting value to 5V effectively "turns off" velocity gates, making all gates the same value. |
| `Velocity` | The disting NT output/bus where the velocity value for the current step of this playhead is sent. |
| `Quant Send` | The disting NT output/bus to "send" the raw, unquantized step value to. |
| `Quant Return` | The disting NT input/bus to act as the "return" bus for a quantized value. When using `Quant Return`, this is the value that will be sent to the `Value` output/bus. |

So why would you want to use a send/return for quantizing your values, rather than just sticking a quantizer after the `Value` output?  The answer is when you are using `Glide`.  If you quantize the value after the sequencer, then any glide is just going to be swallowed by your quantizer, or its going to glide in discrete steps, and not smoothly (which may be cool, but may also not be what you want).  By using a send/return to send pitch information out to your quantizer, the sequencer can ask the quantizer to tell it the quantized value of the next step, and then smoothly glide the pitch there.  If this is unimportant to you, just send `Value` so your quantizer, and don't use the extra send/return channels.

### Playhead Options

| Parameter | Description |
|--------|--------|
| `Gate Length From` | This is how the playhead derives its gate length.  You can set it to `Clock`, which means the maximum gate length is taken from the period of your clock (so as your clock gets faster, your gate lengths become shorter, and vice-versa).  For when you want to be sure gates don't "bleed over" into the next step.  The other option is `Max Gate Len` which means your maximum gate length is taken from that parameter, below.  When using this option, your gate length is going to be the same, regardless of your clock speed.  So if you speed the clock up where the period is shorter than your `Max Gate Len`, your gates will bleed over into one another, if their lengths are not further attenuated by the `Gate Length` cell attribute. |
| `Max Gate Len` | Only relevant when `Gate Length From` is set to `Max Gate Len`. This is the absolute length of your playhead's gate, in milliseconds. |
| `Gate Atten %` | Only relevant when `Gate Length From` is set to `Clock`. This attenuates the period of your clock to determine your maximum gate length. Final gate length is further attenuated by the `Gate Length` cell attribute. |
| `Humanize %` | Percentage by which each gate position, length, and velocity are varied, to add that "human" element. Range is 0–25%. 0% is tight, and 25% is super loose. |
| `Attenuate Value` | Attenuates the (pre-quantized) value that comes out of this playhead. Useful to contract the range of values returned by this playhead. |
| `Offset Value` | Offsets the (pre-quantized, post-attenuated) value that comes out of this playhead. Useful to shift this playhead up or down in register. |
| `Attenuate Velocity` | Attenuates the velocity that comes out of this playhead. |
| `Offset Velocity` | Offsets the velocity that comes out of this playhead. |
| `Move N` | The number of cells this playhead advances in the given direction. |
| `Rest After N` | The playhead will play this many steps, and then rest on the next step it visits. Useful to keep a sequence from playing on every clock. Zero means no rests occur. |
| `Skip After N` | This playhead advances this many times, and then on the next advance, it skips an extra step. Useful to "shake up" a sequence. |
| `Reset After N` | This playhead will reset to its initial position after this many steps. Use this to turn wild, irregular sequences into something regular. |
| `Inactive Reset` | This playhead will reset to its initial position after 10 seconds of no clock input if set. |

<br>
<br>


# Mod Matrix Expander

## Overview

So what if you want to modulate an attribute of a single cell in the grid?  Normally with the disting NT, everything you want to modulate would be its own parameter of the algorithm.  However the disting NT has a limit on how many parameters a single algorithm can have, and unfortunately if you take the 32 cells and multiply out by the 15 possible attributes, that comes out to way to many parameters.  But it sure would be cool to be able to change all of these attributes with modulation!  Of course you would likely never modulate them all at once, but it would be great to, for example, modulate the `Value` on cells 5 and 7, modulate the `Velocity` on 3 and 11, and modulate the `Direction` on cells 2, 4, 6, and 8.

This is where the Mod Matrix Expander for the Directional Sequencer comes in!  It is a sidecar to the Directional Sequencer, and when you place it just below the Directional Sequencer in the algorithm list, it "links up" to it automatically.  If it is not linked to a Directional Sequencer, the UI will instruct you to move it into place.  However once linked, you will see a simple UI like this:

![alt text](<../images/Image 103.png>)

Each Mod Matrix Expander can target up to 4 attributes of its linked Directional Sequencer, and once targeted, it offers you disting NT parameters you can use to change those values on individual cells, as well as some other fun stuff described below!

To get started, go into the disting NT parameters of the expander, and select one of the 4 matrix pages.  Within that page, the top parameter is the `Target` attribute for that matrix.  In the below image, you can see I've chosen `Value` for Matrix A, `Velocity` for Matrix B, and `Direction` for Matrix C, to match the above scenario.  Matrix D is not assigned to any attribute, so it's ignored.

![alt text](<../images/Image 104.png>)

You can also see from this image, that there is a parameter for each grid cell for the attribute, and you can assign CV/MIDI/I2C mappings to whichever ones you like!  The parameter value stays in sync with the underlying attribute value in the Directional Sequencer, so as long as the two are linked (matrix below sequencer), when you change one, the other changes automatically!

But the expander only allows you to target 4 attributes, and there are 15 total.  So you can chain multiple matrix algorithms, one below the next, to stack up as many attribute values as you want to modulate!

## Parameters

The expander has some other modulation tricks as well.  We will go over all of the parameters now:

| Parameter | Description |
|--------|--------|
| `Target` | This is the attribute you with to target with this mod matrix, as described above. |
| `Cell (1,1)` to `Cell (8,4)` | The 32 grid cells. Use these to modulate the `Value`, `Velocity`, `Direction`, etc. for a specific cell. |
| `Randomize Range A / B` | Minimum and maximum `Target` attribute values applied when using randomize operations. |
| `Change By Max` | Maximum percentage of the attribute range to change `Target` values by when using "randomly change" operations. |
| `Action Cell` | Selects which cell a single-cell modulation trigger should affect. |
| `Scramble All` | Trigger that shuffles all `Target` attribute values across the grid (values are mixed, not altered). |
| `Invert All` | Trigger that inverts all `Target` attribute values within their ranges (e.g., velocity 10 → 117, probability 25% → 75%). |
| `Randomize All` | Trigger that randomizes all `Target` attribute values within `Randomize Range A` and `Randomize Range B`. |
| `Randomly Change All` | Trigger that randomly adjusts all `Target` attribute values by up to `Change By Max` percent. |
| `Invert` | Trigger that inverts the `Target` attribute value of the selected `Action Cell` within its range. |
| `Randomize` | Trigger that randomizes the `Target` attribute value of the selected `Action Cell` within the randomize range. |
| `Randomly Change` | Trigger that randomly changes the `Target` attribute value of the selected `Action Cell` by up to `Change By Max` percent. |
| `Swap With Neighbor` | Trigger that swaps the selected `Action Cell`'s `Target` attribute value with one of its neighbors (edges wrap around). |
| `Rotate Row` | Trigger that rotates all `Target` attribute values in the `Action Cell`'s row by one position. |
| `Rotate Column` | Trigger that rotates all `Target` attribute values in the `Action Cell`'s column by one position. |
