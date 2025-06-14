# Scanner3D (V2)

You can find the explaination of the process
[here](https://ulysse_durand.gitlab.io/scanner3d/v2nb)

## Input images

To scan a 3d object, get `NB_IMAGES` images of the object's horizontal shadows
uniformly around the object on its horizontal plane. Try to preprocess it to
have black (in shadow) or white (not in shadow) pixel values.

Then put these views in the `data/` folder, with name `X.png` for X the ID of
the image. X should correspond to the angle of the shadow with the formula 
$$angle = \frac{X * 360}{\text{NB\_IMAGES}}$$
in degrees.

## Execute

Then change the parameters in the `Makefile` `run` target.

- `N` the number of views of the object's shadows
- `H` the height of the input images
- `W` the width of the input images

Optional:
- `x` the x resolution of the result
- `y` the y resolution of the result
- `z` the z resolution of the result

Finally, run
```
make docker-build && make docker-run
```

## How it works

We start with a 3D Voxel grid that represents the object. We project the shadow
with its corresponding angle on the object and we remove all the voxels out of
the shadow. After doing that with all the shadows we obtain the intersection of
them. Finally, applying the Marching Cubes algorithm reconstructs a mesh we
export to a .obj file.