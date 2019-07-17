# The V-HACD library decomposes a 3D surface into a set of "near" convex parts.

:pushpin: The goal of my fork is to create a cross-platform, lightweight python wrapper for V-HACD.

I refer to the [V_HACD](https://github.com/bulletphysics/bullet3/blob/master/Extras/VHACD/test/src/main.cpp) module in the bullet physics simulation engine a lot. But it seems like the V-HACD functionalities are not exposed in the pybullet interface.

<img src="doc/acd.png" alt="Approximate convex decomposition of Camel" width="500"/>

# Installation

## Build from source
### Windows
Prerequisites: install [cmake](https://cmake.org/)

```powershell
cd install
python -m run --cmake
# -- Build files have been written to: <path>/v-hacd/build/win32
cd ../build/win32
cmake --build .
```

Then you should be able to find the built binary at `<path>\v-hacd\build\win32\test\Debug\testVHACD.exe`.

# Command line usage examples

## Windows

Lower quality but faster:
* `testVHACD.exe --input <input>.obj --output <output>.wrl --log log.txt --resolution 100000 --depth 20 --concavity 0.0025 --planeDownsampling 4 --convexhullDownsampling 4 --alpha 0.05 --beta 0.05 --gamma 0.00125 --pca 0 --mode 0 --maxNumVerticesPerCH 64 --minVolumePerCH 0.0`

High quality but slow:
* `testVHACD.exe --input <input>.obj --output <output>.wrl --log log.txt --resolution 8000000 --depth 20 --concavity 0.0025 --planeDownsampling 4 --convexhullDownsampling 4 --alpha 0.05 --beta 0.05 --gamma 0.00125 --pca 0 --mode 0 --maxNumVerticesPerCH 64 --minVolumePerCH 0.0`

The main difference is on the `--resolution` parameter, increased from `100000` to `8000000`.

## OSX, Linux

Coming soon.

<!-- # Installing the Package

1. Clone this Github repository
1. You may either rebuild the binaries for your machine or use the provided binaries.  Here we will assume that you will use the provided binaries in the v-hacd/bin directory.
1. Copy the Python script in v-hacd/add-ons/blender/object_vhacd.py to your Blender addons directory.  For Blender 2.78 this directory will be "Blender Foundation/Blender/2.78/scripts/addons/".  You're at the right place if you see other scripts prefixed with object\_.
1. The addon must be enabled before use.  After copying the script to the addons directory, open Blender and navigate to File > User Preferences > Add-ons.  Object: V-HACD will be featured on the list.  Check its mark to enable the addon and save user settings.

# Using the Addon

1. After you have enabled the addon, the V-HACD menu will appear in the object menu when an object is selected.
1. Go to this menu and select a preset (or leave the path presets).
1. Select your VHACD path by selecting the "Open File" button next to the VHACD path, which should currently be blank.
  1. Navigate to the directory in which you cloned this repository and find the appropriate executable in the bin folder for your operating systme.
  1. If you have a video card that supports OpenCL, this will be bin.
  1. If you don't have a video card or your video card does not support OpenCL, the appropriate executable for you will be bin-no-ocl (no OpenCL).
1. Select the V-HACD button at the button of the panel.  You will be presented with some options.
1. Modify the options as desired and select "OK."
1. Note that the processing may take some time.  Increasing voxel resolution will particularly increase runtime. -->

# Parameters
| cmd Parameter | Description | Default value | Range |
| ------------- | ------------- | ------------- | ---- |
| `--input` | wavefront .obj input file name | - | - |
| `--output` | VRML 2.0 .wrl output file name | - | - |
| `--log` | log file name | - | - |
| `--resolution` | maximum number of voxels generated during the voxelization stage	| 100,000 | 10,000-64,000,000 |
| `--depth` |	maximum number of clipping stages. During each split stage, all the model parts (with a concavity higher than the user defined threshold) are clipped according the "best" clipping plane | 20 | 1-32 |
| `--concavity` |	maximum concavity |	0.0025 | 0.0-1.0 |
| `--planeDownsampling` |	controls the granularity of the search for the "best" clipping plane | 4 | 1-16 |
| `--convexhullDownsampling` | controls the precision of the convex-hull generation process during the clipping plane selection stage | 4 | 1-16 |
| `--alpha` | controls the bias toward clipping along symmetry planes | 0.05 | 0.0-1.0 |
| `--beta` | controls the bias toward clipping along revolution axes | 0.05 | 0.0-1.0 |
| `--gamma` |	maximum allowed concavity during the merge stage | 0.00125 | 0.0-1.0 |
| `--delta` |  Controls the bias toward maximaxing local concavity | 0.05 | 0.0-1.0 |
| `--pca` |	enable/disable normalizing the mesh before applying the convex decomposition | 0 | 0, 1 |
| `--mode` | 0: voxel-based approximate convex decomposition, 1: tetrahedron-based approximate convex decomposition | 0 | 0, 1 |
| `--maxNumVerticesPerCH` |	controls the maximum number of triangles per convex-hull | 64 | 4-1024 |
| `--minVolumePerCH` | controls the adaptive sampling of the generated convex-hulls | 0.0001 | 0.0-0.01 |
| `--convexhullApproximation` | Enable/disable approximation when computing convex-hulls | 1 | 0, 1 |
| `--oclAcceleration` | Enable/disable OpenCL acceleration | 0 | 0, 1 |

# Why do we need approximate convex decomposition?

Collision detection is essential for [realistic physical interactions](https://www.youtube.com/watch?v=oyjE5L4-1lQ) in video games and computer animation. In order to ensure real-time interactivity with the player/user, video game and 3D modeling software developers usually approximate the 3D models composing the scene (e.g. animated characters, static objects...) by a set of simple convex shapes such as ellipsoids, capsules or convex-hulls. In practice, these simple shapes provide poor approximations for concave surfaces and generate false collision detection.


<img src="doc/chvsacd.png" alt="Convex-hull vs. ACD" width="500"/>

A second approach consists in computing an exact convex decomposition of a surface S, which consists in partitioning it into a minimal set of convex sub-surfaces. Exact convex decomposition algorithms are NP-hard and non-practical since they produce a high number of clusters. To overcome these limitations, the exact convexity constraint is relaxed and an approximate convex decomposition of S is instead computed. Here, the goal is to determine a partition of the mesh triangles with a minimal number of clusters, while ensuring that each cluster has a concavity lower than a user defined threshold.

<img src="doc/ecdvsacd.png" alt="ECD vs. ACD" width="500"/>


<!-- # More approximate convex decomposition results
![V-HACD Results (1/4)](https://raw.githubusercontent.com/kmammou/v-hacd/master/doc/snapshots_1.png)
![V-HACD Results (2/4)](https://raw.githubusercontent.com/kmammou/v-hacd/master/doc/snapshots_2.png)
![V-HACD Results (3/4)](https://raw.githubusercontent.com/kmammou/v-hacd/master/doc/snapshots_3.png)
![V-HACD Results (4/4)](https://raw.githubusercontent.com/kmammou/v-hacd/master/doc/snapshots_4.png) -->

# Citation

The author of the [original V-HACD](https://github.com/kmammou/v-hacd) is [Khaled Mammou](http://www.khaledmammou.com/index.htm)
 ([@kmammou](https://github.com/kmammou)). In his [blog post](http://kmamou.blogspot.com/2011/10/hacd-hierarchical-approximate-convex.html), you will find more information on the history of the birth of this project.

Details of the algorithm used in V-HACD can be found in the following paper:

    @inproceedings{mamou2009simple,
      title={A simple and efficient approach for 3D mesh approximate convex decomposition},
      author={Mamou, Khaled and Ghorbel, Faouzi},
      booktitle={2009 16th IEEE international conference on image processing (ICIP)},
      pages={3501--3504},
      year={2009},
      organization={IEEE}
    }
