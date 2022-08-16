# Mira (heavily WIP)
D3D12 Renderer using modern bindless workflow using Direct Descriptor Access.  
Requires Shader Model 6.6.  
  
The render device API is quite bound to D3D12 right now, but it is a learning exercise so that once I decide to implement Vulkan,
I can reason about my mistakes more concretely and change the user-facing API accordingly (which will be a lot of work if the 
coupling to the backend is really tight).