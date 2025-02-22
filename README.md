# Fluid Simulation
## Lattice Boltzmann Method ( LBM )

<p align="center">
  <img src="https://github.com/user-attachments/assets/95214616-9cde-4c26-ab0f-af06f671033e" height="160" width="400">
  <img src="https://github.com/user-attachments/assets/24f21671-62f4-4fb1-8f71-8f834e3fce89" height="160" width="400">
  <br>
  <img src="https://github.com/user-attachments/assets/17f60251-462b-441c-a8e6-456738ee8cfc" height="160" width="400">
  <img src="https://github.com/user-attachments/assets/31d7a299-d3e2-4f0e-9794-5e444115d28e" height="160" width="400">
  <br>
  <figcaption>Caption for the whole set of images</figcaption>
</p>

### Steps
#### (1) Grid Initialization
We need to initialize every cell on the grid with a non-zero density.
#### (2) Set Inlet
In this step we set the velocity distribution across the inlet. We can choose from a variety of distributions, such as uniform or Poiseuille flow. In addition, we can add
perturbation to the inlet, either temporal or spatial or both.
#### (3) Streaming Step
The streaming step propagates f-values to neighboring cells. 
Streaming only applies to fluid cells, so obstacle cells are ignored.
For simplicity and efficiency, f-values form fluid cells are allowed to stream into obstacle cells. This results in obstacle cells having non-zero f-values. Obstacle cells
are not allowed to have non-zero f-values, but this discrepancy will be handled later.

First we stream the inlet. In the case of inlet cells, we ignore any f-values that are streamed towards the left ( as this is off-grid ). 

Next we stream the interior of the tunnel. The f-values of these cells are streamed in all directions. 

Lastly we stream the outlet. This is the opposite of inlet streaming in that we ignore f-values that stream to the right.

#### (4) Flush Tunnel Walls
This step resolves the direpancy in our previous step: that of obstacle cells having
non-zero f-values. Non-zero f-values in obstacles cells first have their direction 
inverted and then propagated back to their original cell.

#### (5) Handle Obstacle

If there are any obstacles inisde the tunnel interior, we flush these obstacle cells of any non-zero f-values in the same way as the tunnel walls.

#### (6) Collision Step
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
