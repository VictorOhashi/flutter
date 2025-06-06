// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_GEOMETRY_GEOMETRY_H_
#define FLUTTER_IMPELLER_ENTITY_GEOMETRY_GEOMETRY_H_

#include "impeller/core/formats.h"
#include "impeller/core/vertex_buffer.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/stroke_parameters.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/vertex_buffer_builder.h"

namespace impeller {

[[maybe_unused]]
static constexpr Scalar kMinStrokeSize = 1.0f;

struct GeometryResult {
  enum class Mode {
    /// The geometry has no overlapping triangles.
    kNormal,
    /// The geometry may have overlapping triangles. The geometry should be
    /// stenciled with the NonZero fill rule.
    kNonZero,
    /// The geometry may have overlapping triangles. The geometry should be
    /// stenciled with the EvenOdd fill rule.
    kEvenOdd,
    /// The geometry may have overlapping triangles, but they should not
    /// overdraw or cancel each other out. This is a special case for stroke
    /// geometry.
    kPreventOverdraw,
  };

  PrimitiveType type = PrimitiveType::kTriangleStrip;
  VertexBuffer vertex_buffer;
  Matrix transform;
  Mode mode = Mode::kNormal;
};

static const GeometryResult kEmptyResult = {
    .vertex_buffer =
        {
            .index_type = IndexType::kNone,
        },
};

class Geometry {
 public:
  virtual ~Geometry() {}

  static std::unique_ptr<Geometry> MakeFillPath(
      const flutter::DlPath& path,
      std::optional<Rect> inner_rect = std::nullopt);

  static std::unique_ptr<Geometry> MakeStrokePath(
      const flutter::DlPath& path,
      const StrokeParameters& stroke = {});

  static std::unique_ptr<Geometry> MakeCover();

  static std::unique_ptr<Geometry> MakeRect(const Rect& rect);

  static std::unique_ptr<Geometry> MakeOval(const Rect& rect);

  static std::unique_ptr<Geometry> MakeLine(const Point& p0,
                                            const Point& p1,
                                            const StrokeParameters& stroke);

  static std::unique_ptr<Geometry> MakeCircle(const Point& center,
                                              Scalar radius);

  static std::unique_ptr<Geometry> MakeStrokedCircle(const Point& center,
                                                     Scalar radius,
                                                     Scalar stroke_width);

  static std::unique_ptr<Geometry> MakeFilledArc(const Rect& oval_bounds,
                                                 Degrees start,
                                                 Degrees sweep,
                                                 bool include_center);

  static std::unique_ptr<Geometry> MakeStrokedArc(
      const Rect& oval_bounds,
      Degrees start,
      Degrees sweep,
      const StrokeParameters& stroke);

  static std::unique_ptr<Geometry> MakeRoundRect(const Rect& rect,
                                                 const Size& radii);

  static std::unique_ptr<Geometry> MakeRoundSuperellipse(const Rect& rect,
                                                         Scalar corner_radius);

  virtual GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                           const Entity& entity,
                                           RenderPass& pass) const = 0;

  virtual GeometryResult::Mode GetResultMode() const;

  virtual std::optional<Rect> GetCoverage(const Matrix& transform) const = 0;

  /// @brief Compute an alpha value to simulate lower coverage of fractional
  ///        pixel strokes.
  static Scalar ComputeStrokeAlphaCoverage(const Matrix& entity,
                                           Scalar stroke_width);

  /// @brief    Determines if this geometry, transformed by the given
  ///           `transform`, will completely cover all surface area of the given
  ///           `rect`.
  ///
  ///           This is a conservative estimate useful for certain
  ///           optimizations.
  ///
  /// @returns  `true` if the transformed geometry is guaranteed to cover the
  ///           given `rect`. May return `false` in many undetected cases where
  ///           the transformed geometry does in fact cover the `rect`.
  virtual bool CoversArea(const Matrix& transform, const Rect& rect) const;

  virtual bool IsAxisAlignedRect() const;

  virtual bool CanApplyMaskFilter() const;

  virtual Scalar ComputeAlphaCoverage(const Matrix& transform) const {
    return 1.0;
  }

  static GeometryResult ComputePositionGeometry(
      const ContentContext& renderer,
      const Tessellator::VertexGenerator& generator,
      const Entity& entity,
      RenderPass& pass);
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_GEOMETRY_GEOMETRY_H_
