//
// plume
//
// Copyright (c) 2024 renderbag and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file for details.
//

#pragma once

#include <unordered_set>

namespace plume {
    struct RenderDescriptorSetBuilder {
        std::list<std::vector<const RenderSampler *>> samplerPointerVectorList;
        std::vector<RenderDescriptorRange> descriptorRanges;
        RenderDescriptorSetDesc descriptorSetDesc;
        bool open = false;
        uint32_t setIndex = 0;

        RenderDescriptorSetBuilder() = default;

        void begin() {
            assert(!open && "Builder must be closed.");

            descriptorSetDesc = RenderDescriptorSetDesc();

            samplerPointerVectorList.clear();
            descriptorRanges.clear();

            open = true;
            setIndex = 0;
        }

        uint32_t addConstantBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::CONSTANT_BUFFER, binding, count, nullptr));
        }

        uint32_t addFormattedBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::FORMATTED_BUFFER, binding, count, nullptr));
        }

        uint32_t addReadWriteFormattedBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::READ_WRITE_FORMATTED_BUFFER, binding, count, nullptr));
        }

        uint32_t addTexture(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::TEXTURE, binding, count, nullptr));
        }

        uint32_t addReadWriteTexture(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::READ_WRITE_TEXTURE, binding, count, nullptr));
        }

        uint32_t addSampler(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::SAMPLER, binding, count, nullptr));
        }

        uint32_t addImmutableSampler(uint32_t binding, const RenderSampler *immutableSampler) {
            assert(immutableSampler != nullptr);

            return addImmutableSampler(binding, &immutableSampler);
        }

        uint32_t addImmutableSampler(uint32_t binding, const RenderSampler **immutableSampler, uint32_t count = 1) {
            assert(immutableSampler != nullptr);

            samplerPointerVectorList.emplace_back(std::vector<const RenderSampler *>(immutableSampler, immutableSampler + count));
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::SAMPLER, binding, count, samplerPointerVectorList.back().data()));
        }

        uint32_t addStructuredBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::STRUCTURED_BUFFER, binding, count, nullptr));
        }

        uint32_t addReadWriteStructuredBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::READ_WRITE_STRUCTURED_BUFFER, binding, count, nullptr));
        }

        uint32_t addByteAddressBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER, binding, count, nullptr));
        }

        uint32_t addReadWriteByteAddressBuffer(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER, binding, count, nullptr));
        }

        uint32_t addAccelerationStructure(uint32_t binding, uint32_t count = 1) {
            return addRange(RenderDescriptorRange(RenderDescriptorRangeType::ACCELERATION_STRUCTURE, binding, count, nullptr));
        }

        uint32_t addRange(const RenderDescriptorRange &range) {
            assert(open && "Builder must be open.");

            uint32_t returnValue = setIndex;
            descriptorRanges.emplace_back(range);
            descriptorSetDesc.descriptorRangesCount++;
            setIndex += range.count;
            return returnValue;
        }

        void end(bool lastRangeIsBoundless = false, uint32_t boundlessRangeSize = 0) {
            assert(open && "Builder must be open.");

            descriptorSetDesc.lastRangeIsBoundless = lastRangeIsBoundless;
            descriptorSetDesc.boundlessRangeSize = boundlessRangeSize;
            descriptorSetDesc.descriptorRanges = descriptorRanges.data();
            open = false;
        }

        std::unique_ptr<RenderDescriptorSet> create(RenderDevice *device) const {
            assert(!open && "Builder must be closed.");

            return device->createDescriptorSet(descriptorSetDesc);
        }
    };

    struct RenderDescriptorSetBase {
        RenderDescriptorSetBuilder builder;
        std::unique_ptr<RenderDescriptorSet> descriptorSet;

        void create(RenderDevice *device) {
            descriptorSet = builder.create(device);
        }

        RenderDescriptorSet *get() const {
            return descriptorSet.get();
        }

        void setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, uint64_t bufferSize = 0, const RenderBufferStructuredView *bufferStructuredView = nullptr, const RenderBufferFormattedView *bufferFormattedView = nullptr) {
            descriptorSet->setBuffer(descriptorIndex, buffer, bufferSize, bufferStructuredView, bufferFormattedView);
        }

        void setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, uint64_t bufferSize, const RenderBufferStructuredView &bufferStructuredView) {
            descriptorSet->setBuffer(descriptorIndex, buffer, bufferSize, &bufferStructuredView);
        }

        void setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, uint64_t bufferSize, const RenderBufferFormattedView *bufferFormattedView) {
            descriptorSet->setBuffer(descriptorIndex, buffer, bufferSize, nullptr, bufferFormattedView);
        }

        void setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, const RenderBufferStructuredView &bufferStructuredView) {
            descriptorSet->setBuffer(descriptorIndex, buffer, 0, &bufferStructuredView);
        }

        void setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, const RenderBufferFormattedView *bufferFormattedView) {
            descriptorSet->setBuffer(descriptorIndex, buffer, 0, nullptr, bufferFormattedView);
        }

        void setTexture(uint32_t descriptorIndex, const RenderTexture *texture, const RenderTextureLayout textureLayout, const RenderTextureView *textureView = nullptr) {
            descriptorSet->setTexture(descriptorIndex, texture, textureLayout, textureView);
        }

        void setSampler(uint32_t descriptorIndex, const RenderSampler *sampler) {
            descriptorSet->setSampler(descriptorIndex, sampler);
        }

        void setAccelerationStructure(uint32_t descriptorIndex, const RenderAccelerationStructure *accelerationStructure) {
            descriptorSet->setAccelerationStructure(descriptorIndex, accelerationStructure);
        }
    };

    struct RenderDescriptorSetInclusionFilter {
        const uint32_t *bindings = nullptr;
        uint32_t bindingsCount = 0;
    };
    
    struct RenderPipelineLayoutBuilder {
        std::vector<RenderPushConstantRange> pushConstantRanges;
        std::list<std::vector<const RenderSampler *>> samplerPointerVectorList;
        std::vector<RenderDescriptorRange> descriptorRanges;
        std::vector<RenderDescriptorSetDesc> descriptorSetDescs;
        std::vector<uint32_t> descriptorRangeIndexPerSet;
        std::vector<RenderRootDescriptorDesc> rootDescriptorDescs;
        RenderPipelineLayoutDesc layoutDesc;
        bool open = false;

        // Start filling the description.
        void begin(bool isLocal = false, bool allowInputLayout = false) {
            assert(!open && "Builder must be closed.");

            layoutDesc = RenderPipelineLayoutDesc();
            layoutDesc.isLocal = isLocal;
            layoutDesc.allowInputLayout = allowInputLayout;

            pushConstantRanges.clear();
            samplerPointerVectorList.clear();
            descriptorRanges.clear();
            descriptorSetDescs.clear();
            descriptorRangeIndexPerSet.clear();
            rootDescriptorDescs.clear();

            open = true;
        }

        // Returns push constant index.
        uint32_t addPushConstant(uint32_t binding, uint32_t set, uint32_t size, RenderShaderStageFlags stageFlags, uint32_t offset = 0) {
            assert(open && "Builder must be open.");

            uint32_t returnValue = layoutDesc.pushConstantRangesCount;
            pushConstantRanges.emplace_back(RenderPushConstantRange(binding, set, offset, size, stageFlags));
            layoutDesc.pushConstantRangesCount++;
            return returnValue;
        }

        // Returns set index.
        uint32_t addDescriptorSet(const RenderDescriptorSetDesc &descriptorSetDesc) {
            assert(open && "Builder must be open.");

            uint32_t returnValue = layoutDesc.descriptorSetDescsCount;
            descriptorRangeIndexPerSet.emplace_back(uint32_t(descriptorRanges.size()));
            descriptorSetDescs.emplace_back(descriptorSetDesc);

            for (uint32_t j = 0; j < descriptorSetDesc.descriptorRangesCount; j++) {
                descriptorRanges.emplace_back(descriptorSetDesc.descriptorRanges[j]);

                // Copy the immutable sampler pointers to a local vector list.
                if (descriptorRanges.back().immutableSampler != nullptr) {
                    const RenderSampler **immutableSampler = descriptorRanges.back().immutableSampler;
                    samplerPointerVectorList.emplace_back(std::vector<const RenderSampler *>(immutableSampler, immutableSampler + descriptorRanges.back().count));
                    descriptorRanges.back().immutableSampler = samplerPointerVectorList.back().data();
                }
            }
            
            layoutDesc.descriptorSetDescsCount++;

            return returnValue;
        }

        // Returns set index.
        uint32_t addDescriptorSet(const RenderDescriptorSetBuilder &descriptorSetBuilder) {
            return addDescriptorSet(descriptorSetBuilder.descriptorSetDesc);
        }

        // Returns set index.
        uint32_t addDescriptorSet(const RenderDescriptorSetBase &descriptorSetBase) {
            return addDescriptorSet(descriptorSetBase.builder);
        }

        // Returns root descriptor index. D3D12 only.
        uint32_t addRootDescriptor(uint32_t shaderRegister, uint32_t registerSpace, RenderRootDescriptorType type) {
            assert(open && "Builder must be open.");

            uint32_t returnValue = layoutDesc.rootDescriptorDescsCount;
            rootDescriptorDescs.emplace_back(shaderRegister, registerSpace, type);
            ++layoutDesc.rootDescriptorDescsCount;

            return returnValue;
        }

        // Finish the description.
        void end() {
            assert(open && "Builder must be open.");

            if (layoutDesc.pushConstantRangesCount > 0) {
                layoutDesc.pushConstantRanges = pushConstantRanges.data();
            }

            if (layoutDesc.descriptorSetDescsCount > 0) {
                for (uint32_t i = 0; i < layoutDesc.descriptorSetDescsCount; i++) {
                    const uint32_t rangeIndex = descriptorRangeIndexPerSet[i];
                    descriptorSetDescs[i].descriptorRanges = &descriptorRanges[rangeIndex];
                }

                layoutDesc.descriptorSetDescs = descriptorSetDescs.data();
            }

            if (layoutDesc.rootDescriptorDescsCount > 0) {
                layoutDesc.rootDescriptorDescs = rootDescriptorDescs.data();
            }

            open = false;
        }

        // Create a pipeline layout with the final description.
        std::unique_ptr<RenderPipelineLayout> create(RenderDevice *device) const {
            assert(!open && "Builder must be closed.");

            return device->createPipelineLayout(layoutDesc);
        }
    };
}
