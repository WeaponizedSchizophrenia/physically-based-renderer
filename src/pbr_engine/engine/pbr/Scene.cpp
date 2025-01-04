#include "pbr/Scene.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/ModelPushConstant.hpp"
#include "pbr/PbrPipeline.hpp"

#include <cstdint>

auto pbr::renderScene(vk::CommandBuffer cmdBuffer, PbrPipeline const& pbrPipeline,
                      Scene const& scene) -> void {
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pbrPipeline.getPipeline());
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               pbrPipeline.getPipelineLayout(), 0,
                               scene.camera->getDescriptorSet(), {});
  for (auto const& mesh : scene.meshes) {
    auto const pushConst = pbr::makeModelPushConstant(
        mesh.transform.position, mesh.transform.rotation, mesh.transform.scale);
    cmdBuffer.pushConstants<ModelPushConstant>(
        pbrPipeline.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, pushConst);

    cmdBuffer.bindVertexBuffers(0, mesh.mesh->getVertexBuffer().getBuffer(), {0});
    cmdBuffer.bindIndexBuffer(mesh.mesh->getIndexBuffer().getBuffer(), 0,
                              vk::IndexType::eUint16);
    for (auto const& primitive : mesh.mesh->getPrimitives()) {
      cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   pbrPipeline.getPipelineLayout(), 1,
                                   primitive.material->getDescriptorSet(), {});
      cmdBuffer.drawIndexed(primitive.indexCount, 1, primitive.firstIndex,
                            static_cast<std::int32_t>(primitive.firstVertex), 0);
    }
  }
}
