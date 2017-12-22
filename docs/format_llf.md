# Little Linkable Format (LLF)
by Brandon Nguyen

## Introduction
The LLF is a new linkable and executable format for the LC-3. It is being
developed to go around the shortcomings of the original .obj format. The
name is a direct reference to the popular Executable and Linkable Format (ELF,
formally known as the Extensible Linking Format).

LLF takes some design cues from ELF while trying to be simple and easy to understand
and implement assemblers and loaders for.

Its features are:
* Magic number for sanity checking (and to prevent users from accidentally loading source code files)
* Embedded symbols so as to keep all relevant data in the file itself
* Support for link and load time symbol resolution
* Code segmentation: can contain multiple segments that can be resolved by the loader
* Support for relocatable code

## Structure
The overall structure involves
* file header, containing the magic number as well as useful metadata
* 

### File Header
| Offset | Size | Field | Purpose |
|:-------|:-----|:------|:--------|
| `0x00` | 4 | Magic number | `0x42 0x4c 0x4c 0x46` (`0x42 'L' 'L' 'F'`) |
| `0x04` | 4 | Version Number | Which version of LLF |

