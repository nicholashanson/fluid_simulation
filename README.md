# Fluid Simulation
## Lattice Boltzmann Methods ( LBM )

![composite_image_with_labels](https://github.com/user-attachments/assets/9aa48893-fd37-4238-aefa-e6a7cc485a34)

### Steps
#### Grid Initialization
We need to initialize every cell on the grid with a non-zero density.
#### Set Inlet
#### Streaming Step
The streaming step propagates f-values to neighboring cells. 
Streaming only applies to fluid cells, so obstacle cells are ignored.
For simplicity and efficiency, f-values form fluid cells are allowed to stream into obstacle cells. This results in obstacle cells having non-zero f-values. Obstacle cells
are not allowed to have non-zero f-values, but this discrepancy will be handled later.

First we stream the inlet. In the case of inlet cells, we ignore any f-values that are streamed towards the left ( as this is off-grid ). 

Next we stream the interior of the tunnel. The f-values of these cells are streamed in all directions. 

Lastly we stream the outlet. This is the opposite of inlet streaming in that we ignore f-values that stream to the right.

#### Flush Tunnel Walls
This step resolves the direpancy in our previous step: that of obstacle cells having
non-zero f-values. Non-zero f-values in obstacles cells first have their direction 
inverted and then propagated back to their original cell.

#### Handle Obstacle

If there are any obstacles inisde the tunnel interior, we flush these obstacle cells of any non-zero f-values in the same way as the tunnel walls.

#### Collision Step
This step helps to ease the fluid towards a state of equilibrium.

$$
f^{\text{eq}}_q = w_q \rho \left( 1 + \frac{\mathbf{e}_q \cdot \mathbf{u}}{c_s^2} + \frac{(\mathbf{e}_q \cdot \mathbf{u})^2}{2 c_s^4} - \frac{|\mathbf{u}|^2}{2 c_s^2} \right)
$$

Where $w_q$ is the weight in the $q$-th direction, $\rho$ is the fluid density, and $e_q = ( e_{qx}, e_{qy} )$ is the discrete velocity vector in the $q$-th direction.
After this step is complete we return to the "Set Inlet" step.

### Parallelization
LBM was designed from inception to run on massively parallel architectures. Almost every part of this algorithm is parallelizable with little or no modification.

I decided to parallelize the collision step using DPC++ ( Intel's implementation of SYCL ). DPC++ is part of the Intel OneAPI Toolkit.

The C++ ABI used by DPC++ is incompatible with GCC and MSVC, so we need to compile our SYCL code into a dll with a C ABI.
