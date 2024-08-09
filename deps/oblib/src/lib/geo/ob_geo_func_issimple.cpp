/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 * This file contains implementation for ob_geo_func_issimple.
 */

#define USING_LOG_PREFIX LIB
#include "lib/geo/ob_geo_dispatcher.h"
#include "lib/geo/ob_geo_func_issimple.h"
#include "lib/geo/ob_geo_utils.h"

using namespace oceanbase::common;
namespace bg = boost::geometry;

namespace oceanbase
{
namespace common
{

template <typename GeoType>
static int eval_issimple_without_strategy(const ObGeometry *g,
                                         const ObGeoEvalCtx &context,
                                         bool &result)
{
  INIT_SUCC(ret);
  const GeoType *geo = nullptr;
  if (!g->is_tree()) {
    geo = reinterpret_cast<GeoType *>(const_cast<char *>(g->val()));
  } else {
    geo = reinterpret_cast<GeoType *>(const_cast<ObGeometry *>(g));
  }
  if (OB_ISNULL(geo)) {
    ret = OB_ERR_NULL_VALUE;
    LOG_WARN("geometry can not be null", K(ret));
  } else {
    // todo
  }
  return ret;
}

template <typename GeoType>
static int eval_issimple_with_strategy(const ObGeometry *g,
                                      const ObGeoEvalCtx &context,
                                      bool &result)
{
  INIT_SUCC(ret);
  
  return ret;
}

class ObGeoFuncIsSimpleImpl : public ObIGeoDispatcher<bool, ObGeoFuncIsSimpleImpl>
{
public:
  ObGeoFuncIsSimpleImpl();
  virtual ~ObGeoFuncIsSimpleImpl() = default;
  // default templates
  OB_GEO_TREE_UNARY_FUNC_DEFAULT(bool, OB_ERR_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS);
  OB_GEO_CART_BINARY_FUNC_DEFAULT(bool, OB_ERR_NOT_IMPLEMENTED_FOR_CARTESIAN_SRS);
  OB_GEO_GEOG_BINARY_FUNC_DEFAULT(bool, OB_ERR_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS);
  OB_GEO_CART_TREE_FUNC_DEFAULT(bool, OB_ERR_NOT_IMPLEMENTED_FOR_CARTESIAN_SRS);
  OB_GEO_GEOG_TREE_FUNC_DEFAULT(bool, OB_ERR_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS);

  // template for unary
  // default cases for cartesian
  template <typename GeoType>
  struct Eval
  {
    static int eval(const ObGeometry *g, const ObGeoEvalCtx &context, bool &result)
    {
      UNUSED(context);
      return eval_issimple_without_strategy<GeoType>(g, context, result);
    }
  };

private:
  template <typename CollectonType>
  static int eval_issimple_gc(const ObGeometry *g,
                              const ObGeoEvalCtx &context,
                              bool &result)
  {
    INIT_SUCC(ret);
    result = true;
    common::ObIAllocator *allocator = context.get_allocator();
    bool is_geog = (g->crs() == oceanbase::common::ObGeoCRS::Geographic);
    typename CollectonType::iterator iter;
    if (OB_ISNULL(allocator)) {
      ret = OB_INVALID_ARGUMENT;
      LOG_WARN("Null allocator", K(ret));
    } else {
      const CollectonType *geo = reinterpret_cast<const CollectonType *>(const_cast<char *>(g->val()));
      iter = geo->begin();
      for (; iter != geo->end() && OB_SUCC(ret) && (result != false); iter++) {
        typename CollectonType::const_pointer sub_ptr = iter.operator->();
        ObGeoType sub_type = geo->get_sub_type(sub_ptr);
        ObGeometry *sub_g = NULL;
        if (OB_FAIL(ObGeoTypeUtil::create_geo_by_type(*allocator, sub_type, is_geog, true, sub_g))) {
          LOG_WARN("failed to create wkb", K(ret), K(sub_type));
        } else {
          // Length is not used, cannot get real length until iter move to the next
          ObString wkb_nosrid(WKB_COMMON_WKB_HEADER_LEN, reinterpret_cast<const char *>(sub_ptr));
          sub_g->set_data(wkb_nosrid);
          sub_g->set_srid(g->get_srid());
        }
        if (OB_SUCC(ret)) {
          ret = eval_wkb_unary(sub_g, context, result);
        }
        // todo
      }
    }
    return ret;
  }
};

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeomCollection, bool)
{
  UNUSED(context);
  return eval_issimple_gc<ObWkbGeomCollection>(g, context, result);
} OB_GEO_FUNC_END;

// geography
OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogPoint, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogPoint>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogLineString, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogLineString>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogPolygon, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogPolygon>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogCollection, bool)
{
  return eval_issimple_gc<ObWkbGeogCollection>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogMultiPoint, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogMultiPoint>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogMultiLineString, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogMultiLineString>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObWkbGeogMultiPolygon, bool)
{
  return eval_issimple_with_strategy<ObWkbGeogMultiPolygon>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_TREE_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObCartesianPolygon, bool)
{
  return eval_issimple_without_strategy<ObCartesianPolygon>(g, context, result);
} OB_GEO_FUNC_END;

OB_GEO_UNARY_TREE_FUNC_BEGIN(ObGeoFuncIsSimpleImpl, ObCartesianMultipolygon, bool)
{
  UNUSED(context);
  return eval_issimple_without_strategy<ObCartesianMultipolygon>(g, context, result);
} OB_GEO_FUNC_END;

int ObGeoFuncIsSimple::eval(const ObGeoEvalCtx &gis_context, bool &result)
{
  return ObGeoFuncIsSimpleImpl::eval_geo_func(gis_context, result);
}

} // sql
} // oceanbase