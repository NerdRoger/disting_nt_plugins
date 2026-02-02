# Window Comparator
An 8-channel window comparator for the disting NT Eurorack module, with loads of related triggers, gates, and values.

## What's a window comparator?
A window comparator, in its most basic form, is a circuit used to determine if a signal is within a specified voltage range, returning a logical TRUE if it is, and a logical FALSE if it is not.  For example, you can set your "window" to be between 2 and 4 volts, and any time your signal is within that range, you basically get a gate for the duration of time that it is.

## Overview
This plugin is basically 8 channels of that, with some extra goodies tacked on that, when using multiple channels, allow you to generate various related (but different) signals, triggers, and gates.

### User Interface and Controls

Let's start by looking at a single comparator first, illustrating the 2-4V window as described above:

![alt text](<../images/Image 201.png>)

In the above image, you can see a single comparator, A.  The default voltage range is -5V to 5V, however you can change that in the parameters.  If you look closely, you can see ruler marks on the channel breaking it up into 1V increments.  The bright box within the channel is the window, currently set to be 2V to 4V.  The signal value is indicated by the bright line, currently showing just around the 0V mark.  Finally, there is the help section at the bottom, describing the controls, which we will cover further down.

Now let's add some more channels, with different window settings, and compare:

![alt text](<../images/Image 202.png>)

You can see I've added 3 more channels, and adjusted the windows of each channel.  Channel A is currently selected, indicated by the bullet to the left, as well as the brighter selection color.  Additionally, you can also see that each window is now being drawn with a block in the center of the window.  This indicates that the mode of the plugin has changed such that, instead of changing the window using its left and right position as in the previous screenshot, we now instead specify the center of the window, and its width.  Finally, you will note that channels B and C also have the window filled in, indicating that the signal is currently within the window, which you can see by looking at the signal marker.

Similar to other areas of the disting NT, each control has reminder text above it reminding you of the controls.  They are:

| Control | Effect |
|--------|--------|
| Left Pot Turn | Adjust the selected comparator's window `Window Left` or `Window Center` position, depending on mode. |
| Right Pot Turn | Adjust the selected comparator's window `Window Right` position or `Window Width`, depending on mode. |
| Left Encoder Turn | Select a comparator channel |
| Right Encoder Quick Press | Lock the UI to prevent accidental changes. |
| Right Encoder Long Press | Change from Bounds editing mode to Center/Width editing mode. |

<br>

## Parameters, Inputs, and Outputs

<b>IMPORTANT</b>:  All trigger and gate outputs will completely overwrite the data on that bus.  However all of the gates and triggers from this plugin will stack, meaning that you can get more frequent triggers, or longer gates, by sending multiple gates/triggers to the same output.

### Global Parameters

| Parameter | Description |
|--------|--------|
| `Range Min` | The minimum of the range for all comparators. |
| `Range Max` | The maximum of the range for all comparators. |
| `Atten Input` | Allows the incoming signal to be attenverted.  This applies to all incoming signals for every channel, however each channel is free to override this if necessary. |
| `Offset Input` | Allows the incoming signal to be offset.  This applies to all incoming signals for every channel, however each channel is free to override this if necessary. |

### Per-Channel Parameters

| Parameter | Description |
|--------|--------|
| `Input` | The disting NT input/bus to take the input signal from. |
| `Window Left` | Left (lower) side of the comparator window.  Changing this will also change `Window Center` and `Window Width` accordingly. |
| `Window Right` | Right (higher) side of the comparator window.  Changing this will also change `Window Center` and `Window Width` accordingly. |
| `Window Center` | Center of the comparator window.  Changing this will also change `Window Left` and `Window Right` accordingly. |
| `Window Width` | Width of the comparator window.  Changing this will also change `Window Left` and `Window Right` accordingly. |
| `Override Global Atten` | If set, this channel specifies its own `Atten Input` and `Offset Input` below. |
| `Atten Input` | Allows the incoming signal to be attenverted.  This overrides the global attenuation values. |
| `Offset Input` | Allows the incoming signal to be offset.  This overrides the global attenuation values. |
| `Value X Contribution` | Amount that this comparator channel contributes to the X aggregate values, as defined below. |
| `Value Y Contribution` | Amount that this comparator channel contributes to the Y aggregate values, as defined below. |
| `Value Z Contribution` | Amount that this comparator channel contributes to the Z aggregate values, as defined below. |
| `Inside Gate` | The disting NT output/bus to send a gate to when the signal is within the window. |
| `Outside Gate` | The disting NT output/bus to send a gate to when the signal is NOT within the window.  Inverse of `Inside Gate`. |
| `Enter Trig` | The disting NT output/bus to send a trigger to when the signal enters the window. |
| `Exit Trig` | The disting NT output/bus to send a trigger to when the signal exits the window. |

### Aggregate Gates

When multiple channels are specified, the following gate outputs become available, which look at the state of all the comparator channels to derive their values.  The number of parameters depend on how many channels there are, but they all follow these patterns:

| Parameter | Description |
|--------|--------|
| `Exactly N Inside Gate` | The disting NT output/bus to send a gate to when EXACTLY N comparators are within their window. |
| `Exactly N Outside Gate` | The disting NT output/bus to send a gate to when EXACTLY N comparators are outside their window. |
| `At Least N Inside Gate` | The disting NT output/bus to send a gate to when AT LEAST N comparators are within their window. |
| `At Least N Outside Gate` | The disting NT output/bus to send a gate to when AT LEAST N comparators are outside their window. |
| `All Inside Gate` | The disting NT output/bus to send a gate to when ALL comparators are within their window. |
| `All Outside Gate` | The disting NT output/bus to send a gate to when ALL comparators are outside their window. |

### Aggregate Values

When multiple channels are specified, the following CV outputs become available, which look at the state of all the comparator channels to derive their values.  These values use the `Value X/Y/Z Contribution` parameters from each channel in their calculations, thereby creating related voltages that you can use in your patch in creative ways.

| Parameter | Description |
|--------|--------|
| `Value X Total Inside` | The disting NT output/bus to send a CV value representing the total of all `Value X Contribution` values for channels that are currently within their windows. |
| `Value X Total Outside` | The disting NT output/bus to send a CV value representing the total of all `Value X Contribution` values for channels that are currently outside their windows. |
| `Value X Avg Inside` | The disting NT output/bus to send a CV value representing the average of all `Value X Contribution` values for channels that are currently within their windows. |
| `Value X Avg Outside` | The disting NT output/bus to send a CV value representing the average of all `Value X Contribution` values for channels that are currently outside their windows. |
| `Value Y Total Inside` | The disting NT output/bus to send a CV value representing the total of all `Value Y Contribution` values for channels that are currently within their windows. |
| `Value Y Total Outside` | The disting NT output/bus to send a CV value representing the total of all `Value Y Contribution` values for channels that are currently outside their windows. |
| `Value Y Avg Inside` | The disting NT output/bus to send a CV value representing the average of all `Value Y Contribution` values for channels that are currently within their windows. |
| `Value Y Avg Outside` | The disting NT output/bus to send a CV value representing the average of all `Value Y Contribution` values for channels that are currently outside their windows. |
| `Value Z Total Inside` | The disting NT output/bus to send a CV value representing the total of all `Value Z Contribution` values for channels that are currently within their windows. |
| `Value Z Total Outside` | The disting NT output/bus to send a CV value representing the total of all `Value Z Contribution` values for channels that are currently outside their windows. |
| `Value Z Avg Inside` | The disting NT output/bus to send a CV value representing the average of all `Value Z Contribution` values for channels that are currently within their windows. |
| `Value Z Avg Outside` | The disting NT output/bus to send a CV value representing the average of all `Value Z Contribution` values for channels that are currently outside their windows. |
