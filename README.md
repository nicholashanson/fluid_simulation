# Fluid Simulation
## Lattice Boltzmann Methods ( LBM )
LBM was designed from inception to run on massively parallel architectures.
### Grid Initialization
We need to initialize every cell on the grid with a non-zero density.
### Set Inlet
### Streaming Step
The streaming step propagates f values to neighboring cells. 
Streaming only applies to fluid cells, so obstacle cells are ignored.
F values form fluid cells are allowed to stream into obstacle cells. Obstacle cells
are not allowed to have non-zero f values, but this discrepancy 
will be handled later.
First we stream the inlet. 
In the case of inlet cells, we ignore  any f value that are streamed towards the left. Next we stream 
the interior of the tunnel. The f values of these cells are streamed 
in all directions. 
Lastly we stream the outlet. This is the opposite of inlet streaming in that
we ignore f values that stream to the right.
### Flush Tunnel Walls
This step resolves the direpancy in our previous step: that of obstacle cells having
non-zero f-values. Non-zero f-values in obstacles cells first have their direction 
inverted and then propagated back to their original cell.
### Collision Step
This step helps to ease the fluid towards a state of equilibrium.
$$
f^{\text{eq}}_q = w_q \rho \left( 1 + \frac{\mathbf{e}_q \cdot \mathbf{u}}{c_s^2} + \frac{(\mathbf{e}_q \cdot \mathbf{u})^2}{2 c_s^4} - \frac{|\mathbf{u}|^2}{2 c_s^2} \right)
$$
Where $w_q$ is the weight in the $q$-th direction, $\rho$ is the fluid density, and $e_q = ( e_{qx}, e_{qy} )$ is the discrete velocity vector in the $q$-th direction.
After this step is complete we return to the "Set Inlet" step.
