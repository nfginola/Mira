#include "PipelineBuilder.h"

namespace mira
{
    GraphicsPipelineBuilder::GraphicsPipelineBuilder()
    {
        RasterizerBuilder rasterizer;
        BlendBuilder blend;
        DepthStencilBuilder ds;
        ds.set_depth_enabled(false);        // Depth testing disabled by default

        set_rasterizer(rasterizer);
        set_blend(blend);
        set_depth_stencil(ds);
    }

    GraphicsPipelineBuilder& mira::GraphicsPipelineBuilder::set_depth_format(DepthFormat format)
    {
        switch (format)
        {
        case DepthFormat::D32:
            m_desc.dsv_format = ResourceFormat::D32_FLOAT;				// Fully qualififies the format: DXGI_FORMAT_R32_TYPELESS
            break;
        case DepthFormat::D32_S8:
            m_desc.dsv_format = ResourceFormat::D32_FLOAT_S8X24_UINT;	// Fully qualififies the format: DXGI_FORMAT_R32G8X24_TYPELESS
            break;
        case DepthFormat::D24_S8:
            m_desc.dsv_format = ResourceFormat::D24_UNORM_S8_UINT;		// Fully qualififies the format: DXGI_FORMAT_R24G8_TYPELESS
            break;
        case DepthFormat::D16:
            m_desc.dsv_format = ResourceFormat::D16_UNORM;				// Fully qualififies the format: DXGI_FORMAT_R16_TYPELESS
            break;
        default:
            assert(false);
        }
        return *this;

    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_shader(const CompiledShader* shader)
    {
        switch (shader->shader_type)
        {
        case ShaderType::Vertex:
            m_desc.vs = shader;
            break;
        case ShaderType::Hull:
            m_desc.hs = shader;
            break;
        case ShaderType::Domain:
            m_desc.ds = shader;
            break;
        case ShaderType::Geometry:
            m_desc.gs = shader;
            break;
        case ShaderType::Pixel:
            m_desc.ps = shader;
            break;
        default:
            assert(false);
        }
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::append_rt_format(ResourceFormat format)
    {
        m_desc.rtv_formats[m_desc.num_render_targets++] = format;
        return *this;
    }
    const GraphicsPipelineDesc& GraphicsPipelineBuilder::build()
    {
        // Sanity checks before returning?
        return m_desc;
    }
}

