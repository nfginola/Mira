# Mira (Under heavy work-in-progress)  
D3D12 Renderer using modern bindless workflow.  
All shader accesses are done using Direct Descriptor Access (Requires Shader Model 6.6). Vertex pulling is also the only way to pass vertex data. 
  
Most of the architecture is handle-based. This is reserved for elements that typically have an immutable nature. If resources are mutable, they are strictly modified through their 'owner' (manager) explicitly.
The aim of this project is to try to work with a 'database-like paradigm', meaning lots of structs-of-arrays (tables) and handles (indices) into them and get comfortable with the pros/cons.  
  
This project can be seen as a successor to the "dx11-tech" project.
  
Gallery:
[Reverse Z](gallery\depth.jpg?raw=true "Reversed Z")
  

