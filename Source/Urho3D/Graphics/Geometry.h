//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/ArrayPtr.h"
#include "../Core/Object.h"
#include "../Graphics/GraphicsDefs.h"

namespace Urho3D
{

class IndexBuffer;
class Ray;
class Graphics;
class VertexBuffer;

/// Defines one or more vertex buffers, an index buffer and a draw range.
class URHO3D_API Geometry : public Object
{
    URHO3D_OBJECT(Geometry, Object);

public:
    /// Construct with one empty vertex buffer.
    explicit Geometry(Context* context);
    /// Destruct.
    ~Geometry() override;

    /// Set number of vertex buffers.
    /// @property
    bool SetNumVertexBuffers(unsigned num);
    /// Set a vertex buffer by index.
    bool SetVertexBuffer(unsigned index, VertexBuffer* buffer);
    /// Set the index buffer.
    /// @property
    void SetIndexBuffer(IndexBuffer* buffer);
    /// Set the draw range.
    bool SetDrawRange(PrimitiveType type, unsigned indexStart, unsigned indexCount, bool getUsedVertexRange = true);
    /// Set the draw range.
    bool SetDrawRange(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned vertexStart, unsigned vertexCount,
        bool checkIllegal = true);
    /// Set the LOD distance.
    /// @property
    void SetLodDistance(float distance);
    /// Override raw vertex data to be returned for CPU-side operations.
    void SetRawVertexData(const SharedArrayPtr<unsigned char>& data, const PODVector<VertexElement>& elements);
    /// Override raw vertex data to be returned for CPU-side operations using a legacy vertex bitmask.
    void SetRawVertexData(const SharedArrayPtr<unsigned char>& data, unsigned elementMask);
    /// Override raw index data to be returned for CPU-side operations.
    void SetRawIndexData(const SharedArrayPtr<unsigned char>& data, unsigned indexSize);
    /// Draw.
    void Draw(Graphics* graphics);

    /// Return all vertex buffers.
    const Vector<SharedPtr<VertexBuffer> >& GetVertexBuffers() const { return vertexBuffers_; }

    /// Return number of vertex buffers.
    /// @property
    unsigned GetNumVertexBuffers() const { return vertexBuffers_.Size(); }

    /// Return vertex buffer by index.
    /// @property{get_vertexBuffers}
    VertexBuffer* GetVertexBuffer(unsigned index) const;

    /// Return the index buffer.
    /// @property
    IndexBuffer* GetIndexBuffer() const { return indexBuffer_; }

    /// Return primitive type.
    /// @property
    PrimitiveType GetPrimitiveType() const { return primitiveType_; }

    /// Return start index.
    /// @property
    unsigned GetIndexStart() const { return indexStart_; }

    /// Return number of indices.
    /// @property
    unsigned GetIndexCount() const { return indexCount_; }

    /// Return first used vertex.
    /// @property
    unsigned GetVertexStart() const { return vertexStart_; }

    /// Return number of used vertices.
    /// @property
    unsigned GetVertexCount() const { return vertexCount_; }

    /// Return LOD distance.
    /// @property
    float GetLodDistance() const { return lodDistance_; }

    /// Return buffers' combined hash value for state sorting.
    unsigned short GetBufferHash() const;
    /// Return raw vertex and index data for CPU operations, or null pointers if not available. Will return data of the first vertex buffer if override data not set.
    void GetRawData(const unsigned char*& vertexData, unsigned& vertexSize, const unsigned char*& indexData, unsigned& indexSize, const PODVector<VertexElement>*& elements) const;
    /// Return raw vertex and index data for CPU operations, or null pointers if not available. Will return data of the first vertex buffer if override data not set.
    void GetRawDataShared(SharedArrayPtr<unsigned char>& vertexData, unsigned& vertexSize, SharedArrayPtr<unsigned char>& indexData,
        unsigned& indexSize, const PODVector<VertexElement>*& elements) const;
    /// Return ray hit distance or infinity if no hit. Requires raw data to be set. Optionally return hit normal and hit uv coordinates at intersect point.
    float GetHitDistance(const Ray& ray, unsigned& subObjectElementIndex, Vector3* outNormal = nullptr, Vector2* outUV = nullptr) const;
    /// Return whether or not the ray is inside geometry.
    bool IsInside(const Ray& ray) const;

    /// Return whether has empty draw range.
    /// @property
    bool IsEmpty() const { return indexCount_ == 0 && vertexCount_ == 0; }

private:
    /// Vertex buffers.
    Vector<SharedPtr<VertexBuffer> > vertexBuffers_;
    /// Index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
    /// Primitive type.
    PrimitiveType primitiveType_;
    /// Start index.
    unsigned indexStart_;
    /// Number of indices.
    unsigned indexCount_;
    /// First used vertex.
    unsigned vertexStart_;
    /// Number of used vertices.
    unsigned vertexCount_;
    /// LOD distance.
    float lodDistance_;
    /// Raw vertex data elements.
    PODVector<VertexElement> rawElements_;
    /// Raw vertex data override.
    SharedArrayPtr<unsigned char> rawVertexData_;
    /// Raw index data override.
    SharedArrayPtr<unsigned char> rawIndexData_;
    /// Raw vertex data override size.
    unsigned rawVertexSize_;
    /// Raw index data override size.
    unsigned rawIndexSize_;
};

}
