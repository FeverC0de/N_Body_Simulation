# Optimizing Cache Locality with Morton Orders in Parallel Barnes-Hut N-Body Simulation
I built this because I love emergent thingies. N Body seemed like a good start in exploring how simple rules create complicated behaviour! 
This also made be overly obsessed with optimization. The goal is to continue optimizing the code to run as many particles as possible.
##  Overview


<img width="1274" height="917" alt="image" src="https://github.com/user-attachments/assets/ee6074a6-db95-4201-91a4-af84cc5b82be" />




Starting from a brute-force N-body implementation (O(N²)), the project moves through:
- Implementing the **Barnes-Hut algorithm** (O(N log N)),
- Leveraging **parallelization with OpenMP**,
- Introducing **Morton ordering for improved memory locality** and predictable tree construction,
- Combining these techniques to enable **real-time simulations with up to ~150,000 bodies**.

The result: up to **15x performance improvement** over the naive brute-force approach.

---

##  Features
- **Brute Force Simulation** – O(N²) baseline with Euler’s method for integration.  
- **Barnes-Hut Algorithm** – Quad/Oct-Tree spatial partitioning to approximate forces.  
- **Parallel Force Calculation** – OpenMP parallelization for acceleration updates.  
- **Parallel Tree Construction** – Morton ordering improves cache locality and enables parallel quad-tree building.  
- **Real-Time Visualization** – Bodies rendered with OpenGL for interactive simulation.  

---

## Performance Highlights
- Brute force method becomes unusable beyond ~10,000 bodies.  
- Barnes-Hut reduces compute time drastically with **O(N log N)** scaling.  
- Morton ordering improves **cache efficiency**, reducing tree build cost by ~202%.  
- Parallel tree construction achieves a **5.14x speedup** compared to unsorted sequential builds.  
- Overall, the optimized implementation allows **real-time simulation of ~150k bodies**.  

---
<img width="1144" height="1034" alt="image" src="https://github.com/user-attachments/assets/08c40881-b723-430c-bc0a-2f78ff6a7a5a" />

##  Technologies Used
- **C++** (core simulation)  
- **OpenGL** (real-time rendering)  
- **OpenMP** (parallelization)  

### Prerequisites
- C++17 (or later)  
- OpenGL (>= 3.3)  
- OpenMP  


## References
- Grama, A., et al. *Scalable Parallel Formulations of the Barnes–Hut Method for N-Body Simulations.* Parallel Computing, 1998.  
- Tokuue, T., & Ishiyama, T. *Optimizing the gravitational tree algorithm for many-core processors.* MNRAS, 2023.  
- StackExchange – [Initializing positions of n-body simulations](https://physics.stackexchange.com/q/749288)  

---






