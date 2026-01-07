# Directional Sequencer

Directional Sequencer is a 2‑D grid-based step sequencer plugin for the disting NT, where each step can be set to move to any of its neighboring steps in an 8x4 grid. It provides multiple independent playheads that move across the grid, and each cell contains various attributes for that step such as value (pitch), probability, ratchets, and others as defined below.

## Overview
When you first add the Directional Sequencer, you can specify how many playheads you would like to use, up to 8.  Choose just a single playhead to get started, then you can respecify to add more when you're ready.  More details below in the Playheads section.  When you first start, the sequencer will look like this:

![alt text](<../images/Image 101.png>)

### User Interface and Controls

As you can see in the image above, the UI is divided into 4 sections:
- On the left, the playhead list.  We just have a single one here.
- In the middle, the grid field.  These are your steps.  Here, you can see that the top-left cell is selected.
- On the right, the per-step attributes of the selected cell.  The UI can only show 5 attributes at a time, but the list scrolls, and there are actually 13 different attributes per cell you can select.
- On the bottom, you can see a help section showing you the controls.  As you scroll through the attributes, this area will change to show you what each of them means.

<br>

Similar to other areas of the disting NT, each control has reminder text above it reminding you of the controls.  They are:
<table>
    <thead>
        <tr>
            <th style="width: 170px;">Control</th>
            <th>Effect</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>Left Encoder Turn</td>
            <td>Change the selected cell in the X direction.</td>
        </tr>
        <tr>
            <td>Right Encoder Turn</td>
            <td>Change the selected cell in the Y direction.</td>
        </tr>
        <tr>
            <td>Left Pot Turn</td>
            <td>Select the playhead. Currently selected head is identified by the play icon next to it.</td>
        </tr>
        <tr>
            <td>Center Pot Turn</td>
            <td>Scroll through the list of cell attributes. Above, you can see the "Direction" attribute is selected.</td>
        </tr>
        <tr>
            <td>Right Pot Turn</td>
            <td>Change the value of the selected attribute for the selected cell.</td>
        </tr>
        <tr>
            <td>Right Encoder Quick Press</td>
            <td>Lock the UI to prevent accidental changes.</td>
        </tr>
        <tr>
            <td>Right Encoder Long Press</td>
            <td>Set the starting step of the selected playhead. Identified by the marquee border around the cell.</td>
        </tr>
        <tr>
            <td>Right Pot Quick Press</td>
            <td>Set the value of the selected attribute to its default value for the selected cell.</td>
        </tr>
        <tr>
            <td>Right Pot Long Press</td>
            <td>Set the value of the selected attribute to its default value for ALL cells.</td>
        </tr>
    </tbody>
</table>

<br>

So using this knowledge of the controls, in the image below, I have navigated down to the Gate Length attribute, then moved thru each cell, assigning a value.  You can see that each cell in the grid shows you a visual representation of the attribute value.  This visual representation will cary from one attribute to the next, but should always give you an idea of all of the values for that attribute at a glance.  You can also see I have moved the selection to the last cell in the third row, and the Gate Length value for that cell is set to 62.  The starting cell for this playhead was changed to be the fourth cell on the first row, as shown by the marquee.  Finally the currently playing step is the 7th step in the first row.

<br>

![alt text](<../images/Image 102.png>)

## Cell Attributes
Each cell carries the following attributes:

<table>
    <thead>
        <tr>
            <th style="width: 150px;">Attribute</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>Direction</code></td>
            <td>The direction to move for the next step. If empty, play continues in the current direction. If a playhead’s initial cell has no value, it moves right (east). Directions that would move outside the grid wrap to the opposite side (Pac‑Man warp).</td>
        </tr>
        <tr>
            <td><code>Value</code></td>
            <td>The step value (typically pitch), modular; range 0–10V.</td>
        </tr>
        <tr>
            <td><code>Velocity</code></td>
            <td>The step velocity. Can be sent to a playhead output and used to create “velocity gates”; range 1–127.</td>
        </tr>
        <tr>
            <td><code>Probability</code></td>
            <td>Percent chance that the step will emit a gate.</td>
        </tr>
        <tr>
            <td><code>Ratchets</code></td>
            <td>Number of evenly‑divided ratchets (based on incoming clock) to play for this step; up to 8. Irregular clocks disable ratcheting.</td>
        </tr>
        <tr>
            <td><code>Rest After</code></td>
            <td>The step will play this many times, then rest on the next visit, then repeat the cycle (e.g., 3 → play 3 times, rest once).</td>
        </tr>
        <tr>
            <td><code>Gate Length</code></td>
            <td>Gate duration for the step as a percentage of either the clock period or the defined max gate length. 0% = no gate; 100% = full legato (tie).</td>
        </tr>
        <tr>
            <td><code>Drift Probability</code></td>
            <td>Percent chance the cell’s value will “drift” (randomly vary) on a visit.</td>
        </tr>
        <tr>
            <td><code>Max Drift</code></td>
            <td>Maximum amount the value can drift (randomly up or down) when drift occurs.</td>
        </tr>
        <tr>
            <td><code>Repeats</code></td>
            <td>If set, the step repeats this many times before advancing.</td>
        </tr>
        <tr>
            <td><code>Glide</code></td>
            <td>How quickly the value glides to the next step, expressed as a percentage of the gate length. 0% = no glide; 100% = glide over the entire gate.</td>
        </tr>
        <tr>
            <td><code>Accumulator Add</code></td>
            <td>Amount to add (or subtract) to the cell value each visit; accumulated across visits to change pitch progressively. Range −1V to +1V.</td>
        </tr>
        <tr>
            <td><code>Accumulator Times</code></td>
            <td>Number of visits the accumulation is applied before the accumulation process resets.</td>
        </tr>
    </tbody>
</table>

## Playheads
You can have up to 8 playheads each moving through the grid.  When you have multiple playheads, you can select them by turning the left pot.  The cell attributes remain the same across all playheads, but there are a number of parameters designed to make each playhead unique!  Each playhead can have its own starting position (chosen using Right Encoder, as above), so that each one can start in different areas of the grid.

Additionally, each playhead has its own routing and option parameters:

### Playhead Routing

<table>
    <thead>
        <tr>
            <th style="width: 150px;">Parameter</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>Clock</code></td>
            <td>Required. The disting NT input/bus to advance the playhead.</td>
        </tr>
        <tr>
            <td><code>Clock Divisor</code></td>
            <td>Only advance the playhead after this many clock triggers. Handy to have multiple playheads running at different rates, all from the same clock.</td>
        </tr>
        <tr>
            <td><code>Clock Offset</code></td>
            <td>A phase offset applied when the clock is divided. For instance, if your divisor is set to 4, and you set the offset to 2, you are advancing this playhead on the off beats.</td>
        </tr>
        <tr>
            <td><code>Reset</code></td>
            <td>The disting NT input/bus to send a trigger to in order to reset the playhead to its initial position.</td>
        </tr>
        <tr>
            <td><code>Value</code></td>
            <td>The disting NT output/bus where the value (pitch) for the current step of this playhead is sent.</td>
        </tr>
        <tr>
            <td><code>Gate</code></td>
            <td>The disting NT output/bus where the gate for the current step of this playhead is sent.</td>
        </tr>
        <tr>
            <td><code>Velocity Gate Min</code></td>
            <td>The minimum amplitude for the gate. The step's velocity is applied on top of this, creating a "velocity gate".</td>
        </tr>
        <tr>
            <td><code>Velocity</code></td>
            <td>The disting NT output/bus where the velocity value for the current step of this playhead is sent.</td>
        </tr>
        <tr>
            <td><code>Quant Send</code></td>
            <td>The disting NT output/bus to "send" the raw, unquantized step value to.</td>
        </tr>
        <tr>
            <td><code>Quant Return</code></td>
            <td>The disting NT input/bus to act as the "return" bus for a quantized value. When using <code>Quant Return</code>, this is the value that will be sent to the <code>Value</code> output/bus.</td>
        </tr>
    </tbody>
</table>

So why would you want to use a send/return for quantizing your values, rather than just sticking a quantizer after the `Value` output?  The answer is when you are using `Glide`.  If you quantize the value after the sequencer, then any glide is just going to be swallowed by your quantizer, or its going to glide in discrete steps, and not smoothly (which may be cool, but may also not be what you want).  By using a send/return to send pitch information out to your quantizer, the sequencer can ask the quantizer to tell it the quantized value of the next step, and then smoothly glide the pitch there.  If this is unimportant to you, just send `Value` so your quantizer, and don't use the extra send/return channels.

### Playhead Options

<table>
    <thead>
        <tr>
            <th><div style="width: 150px;">Parameter</div></th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>Gate Length From</code></td>
            <td>This is how the playhead derives its gate length. You can set it to <code>Clock</code>, which means the maximum gate length is taken from the period of your clock (so as your clock gets faster, your gate lengths become shorter, and vice-versa). For when you want to be sure gates don't "bleed over" into the next step. The other option is <code>Max Gate Len</code> which means your maximum gate length is taken from that parameter, below. When using this option, your gate length is going to be the same, regardless of your clock speed. So if you speed the clock up where the period is shorter than your <code>Max Gate Len</code>, your gates will bleed over into one another, if their lengths are not further attenuated by the <code>Gate Length</code> cell attribute.</td>
        </tr>
        <tr>
            <td><code>Max Gate Len</code></td>
            <td>Only relevant when <code>Gate Length From</code> is set to <code>Max Gate Len</code>. This is the absolute length of your playhead's gate, in milliseconds.</td>
        </tr>
        <tr>
            <td><code>Gate Atten %</code></td>
            <td>Only relevant when <code>Gate Length From</code> is set to <code>Clock</code>. This attenuates the period of your clock to determine your maximum gate length. Final gate length is further attenuated by the <code>Gate Length</code> cell attribute.</td>
        </tr>
        <tr>
            <td><code>Humanize %</code></td>
            <td>Percentage by which each gate position, length, and velocity are varied, to add that "human" element. Range is 0–25%. 0% is tight, and 25% is super loose.</td>
        </tr>
        <tr>
            <td><code>Attenuate Value</code></td>
            <td>Attenuates the (pre-quantized) value that comes out of this playhead. Useful to contract the range of values returned by this playhead.</td>
        </tr>
        <tr>
            <td><code>Offset Value</code></td>
            <td>Offsets the (pre-quantized, post-attenuated) value that comes out of this playhead. Useful to shift this playhead up or down in register.</td>
        </tr>
        <tr>
            <td><code>Attenuate Velocity</code></td>
            <td>Attenuates the velocity that comes out of this playhead.</td>
        </tr>
        <tr>
            <td><code>Offset Velocity</code></td>
            <td>Offsets the velocity that comes out of this playhead.</td>
        </tr>
        <tr>
            <td><code>Move N</code></td>
            <td>The number of cells this playhead advances in the given direction.</td>
        </tr>
        <tr>
            <td><code>Rest After N</code></td>
            <td>The playhead will play this many steps, and then rest on the next step it visits. Useful to keep a sequence from playing on every clock. Zero means no rests occur.</td>
        </tr>
        <tr>
            <td><code>Skip After N</code></td>
            <td>This playhead advances this many times, and then on the next advance, it skips an extra step. Useful to "shake up" a sequence.</td>
        </tr>
        <tr>
            <td><code>Reset After N</code></td>
            <td>This playhead will reset to its initial position after this many steps. Use this to turn wild, irregular sequences into something regular.</td>
        </tr>
        <tr>
            <td><code>Inactive Reset</code></td>
            <td>This playhead will reset to its initial position after 10 seconds of no clock input if set.</td>
        </tr>
    </tbody>
</table>
<br>
<br>


# Mod Matrix Expander

## Overview

So what if you want to modulate an attribute of a single cell in the grid?  Normally with the disting NT, everything you want to modulate would be its own parameter of the algorithm.  However the disting NT has a limit on how many parameters a single algorithm can have, and unfortunately if you take the 32 cells and multiply out by the 13 possible attributes, that comes out to way to many parameters.  But it sure would be cool to be able to change all of these attributes with modulation!  Of course you would likely never modulate them all at once, but it would be great to, for example, modulate the `Value` on cells 5 and 7, modulate the `Velocity` on 3 and 11, and modulate the `Direction` on cells 2, 4, 6, and 8.

This is where the Mod Matrix Expander for the Directional Sequencer comes in!  It is a sidecar to the Directional Sequencer, and when you place it just below the Directional Sequencer in the algorithm list, it "links up" to it automatically.  If it is not linked to a Directional Sequencer, the UI will instruct you to move it into place.  However once linked, you will see a simple UI like this:

![alt text](<../images/Image 103.png>)

Each Mod Matrix Expander can target up to 4 attributes of its linked Directional Sequencer, and once targeted, it offers you disting NT parameters you can use to change those values on individual cells, as well as some other fun stuff described below!

To get started, go into the disting NT parameters of the expander, and select one of the 4 matrix pages.  Within that page, the top parameter is the `Target` attribute for that matrix.  In the below image, you can see I've chosen `Value` for Matrix A, `Velocity` for Matrix B, and `Direction` for Matrix C, to match the above scenario.  Matrix D is not assigned to any attribute, so it's ignored.

![alt text](<../images/Image 104.png>)

You can also see from this image, that there is a parameter for each grid cell for the attribute, and you can assign CV/MIDI/I2C mappings to whichever ones you like!  The parameter value stays in sync with the underlying attribute value in the Directional Sequencer, so as long as the two are linked (matrix below sequencer), when you change one, the other changes automatically!

But the expander only allows you to target 4 attributes, and there are 13 total.  So you can chain multiple matrix algorithms, one below the next, to stack up as many attribute values as you want to modulate!

## Parameters

The expander has some other modulation tricks as well.  We will go over all of the parameters now:

<table>
    <thead>
        <tr>
            <th style="width: 190px;">Parameter</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>Target</code></td>
            <td>This is the attribute you with to target with this mod matrix, as described above.</td>
        </tr>
        <tr>
            <td><code>Cell (1,1)</code> to <code>Cell (8,4)</code></td>
            <td>The 32 grid cells. Use these to modulate the <code>Value</code>, <code>Velocity</code>, <code>Direction</code>, etc. for a specific cell.</td>
        </tr>
        <tr>
            <td><code>Randomize Range A / B</code></td>
            <td>Minimum and maximum <code>Target</code> attribute values applied when using randomize operations.</td>
        </tr>
        <tr>
            <td><code>Change By Max</code></td>
            <td>Maximum percentage of the attribute range to change <code>Target</code> values by when using "randomly change" operations.</td>
        </tr>
        <tr>
            <td><code>Action Cell</code></td>
            <td>Selects which cell a single-cell modulation trigger should affect.</td>
        </tr>
        <tr>
            <td><code>Scramble All</code></td>
            <td>Trigger that shuffles all <code>Target</code> attribute values across the grid (values are mixed, not altered).</td>
        </tr>
        <tr>
            <td><code>Invert All</code></td>
            <td>Trigger that inverts all <code>Target</code> attribute values within their ranges (e.g., velocity 10 → 117, probability 25% → 75%).</td>
        </tr>
        <tr>
            <td><code>Randomize All</code></td>
            <td>Trigger that randomizes all <code>Target</code> attribute values within <code>Randomize Range A</code> and <code>Randomize Range B</code>.</td>
        </tr>
        <tr>
            <td><code>Randomly Change All</code></td>
            <td>Trigger that randomly adjusts all <code>Target</code> attribute values by up to <code>Change By Max</code> percent.</td>
        </tr>
        <tr>
            <td><code>Invert</code></td>
            <td>Trigger that inverts the <code>Target</code> attribute value of the selected <code>Action Cell</code> within its range.</td>
        </tr>
        <tr>
            <td><code>Randomize</code></td>
            <td>Trigger that randomizes the <code>Target</code> attribute value of the selected <code>Action Cell</code> within the randomize range.</td>
        </tr>
        <tr>
            <td><code>Randomly Change</code></td>
            <td>Trigger that randomly changes the <code>Target</code> attribute value of the selected <code>Action Cell</code> by up to <code>Change By Max</code> percent.</td>
        </tr>
        <tr>
            <td><code>Swap With Neighbor</code></td>
            <td>Trigger that swaps the selected <code>Action Cell</code>'s <code>Target</code> attribute value with one of its neighbors (edges wrap around).</td>
        </tr>
        <tr>
            <td><code>Rotate Row</code></td>
            <td>Trigger that rotates all <code>Target</code> attribute values in the <code>Action Cell</code>'s row by one position.</td>
        </tr>
        <tr>
            <td><code>Rotate Column</code></td>
            <td>Trigger that rotates all <code>Target</code> attribute values in the <code>Action Cell</code>'s column by one position.</td>
        </tr>
    </tbody>
</table>
