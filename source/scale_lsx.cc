/*
 *  Copyright 2016 The LibYuv Project Authors. All rights reserved.
 *
 *  Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "libyuv/scale_row.h"

#if !defined(LIBYUV_DISABLE_LSX) && defined(__loongarch_sx)
#include "libyuv/loongson_intrinsics.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

void ScaleARGBRowDown2_LSX(const uint8_t* src_argb,
                           ptrdiff_t src_stride,
                           uint8_t* dst_argb,
                           int dst_width) {
  int x;
  int len = dst_width >> 2;
  (void)src_stride;
  __m128i src0, src1, dst0;

  for (x = 0; x < len; x++) {
    DUP2_ARG2(__lsx_vld, src_argb, 0, src_argb, 16, src0, src1);
    dst0 = __lsx_vpickod_w(src1, src0);
    __lsx_vst(dst0, dst_argb, 0);
    src_argb += 32;
    dst_argb += 16;
  }
}

void ScaleARGBRowDown2Linear_LSX(const uint8_t* src_argb,
                                 ptrdiff_t src_stride,
                                 uint8_t* dst_argb,
                                 int dst_width) {
  int x;
  int len = dst_width >> 2;
  (void)src_stride;
  __m128i src0, src1, tmp0, tmp1, dst0;

  for (x = 0; x < len; x++) {
    DUP2_ARG2(__lsx_vld, src_argb, 0, src_argb, 16, src0, src1);
    tmp0 = __lsx_vpickev_w(src1, src0);
    tmp1 = __lsx_vpickod_w(src1, src0);
    dst0 = __lsx_vavgr_bu(tmp1, tmp0);
    __lsx_vst(dst0, dst_argb, 0);
    src_argb += 32;
    dst_argb += 16;
  }
}

void ScaleARGBRowDown2Box_LSX(const uint8_t* src_argb,
                              ptrdiff_t src_stride,
                              uint8_t* dst_argb,
                              int dst_width) {
  int x;
  int len = dst_width >> 2;
  const uint8_t* s = src_argb;
  const uint8_t* t = src_argb + src_stride;
  __m128i src0, src1, src2, src3, tmp0, tmp1, tmp2, tmp3, dst0;
  __m128i reg0, reg1, reg2, reg3;
  __m128i shuff = {0x0703060205010400, 0x0F0B0E0A0D090C08};

  for (x = 0; x < len; x++) {
    DUP2_ARG2(__lsx_vld, s, 0, s, 16, src0, src1);
    DUP2_ARG2(__lsx_vld, t, 0, t, 16, src2, src3);
    DUP4_ARG3(__lsx_vshuf_b, src0, src0, shuff, src1, src1, shuff, src2, src2,
              shuff, src3, src3, shuff, tmp0, tmp1, tmp2, tmp3);
    DUP4_ARG2(__lsx_vhaddw_hu_bu, tmp0, tmp0, tmp1, tmp1, tmp2, tmp2, tmp3,
              tmp3, reg0, reg1, reg2, reg3);
    DUP2_ARG2(__lsx_vsadd_hu, reg0, reg2, reg1, reg3, reg0, reg1);
    dst0 = __lsx_vsrarni_b_h(reg1, reg0, 2);
    __lsx_vst(dst0, dst_argb, 0);
    s += 32;
    t += 32;
    dst_argb += 16;
  }
}

void ScaleARGBRowDownEven_LSX(const uint8_t* src_argb,
                              ptrdiff_t src_stride,
                              int32_t src_stepx,
                              uint8_t* dst_argb,
                              int dst_width) {
  int x;
  int len = dst_width >> 2;
  int32_t stepx = src_stepx << 2;
  (void)src_stride;
  __m128i dst0, dst1, dst2, dst3;

  for (x = 0; x < len; x++) {
    dst0 = __lsx_vldrepl_w(src_argb, 0);
    src_argb += stepx;
    dst1 = __lsx_vldrepl_w(src_argb, 0);
    src_argb += stepx;
    dst2 = __lsx_vldrepl_w(src_argb, 0);
    src_argb += stepx;
    dst3 = __lsx_vldrepl_w(src_argb, 0);
    src_argb += stepx;
    __lsx_vstelm_w(dst0, dst_argb, 0, 0);
    __lsx_vstelm_w(dst1, dst_argb, 4, 0);
    __lsx_vstelm_w(dst2, dst_argb, 8, 0);
    __lsx_vstelm_w(dst3, dst_argb, 12, 0);
    dst_argb += 16;
  }
}

void ScaleARGBRowDownEvenBox_LSX(const uint8_t* src_argb,
                                 ptrdiff_t src_stride,
                                 int src_stepx,
                                 uint8_t* dst_argb,
                                 int dst_width) {
  int x;
  int len = dst_width >> 2;
  int32_t stepx = src_stepx << 2;
  const uint8_t* next_argb = src_argb + src_stride;
  __m128i src0, src1, src2, src3;
  __m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  __m128i reg0, reg1, dst0;

  for (x = 0; x < len; x++) {
    tmp0 = __lsx_vldrepl_d(src_argb, 0);
    src_argb += stepx;
    tmp1 = __lsx_vldrepl_d(src_argb, 0);
    src_argb += stepx;
    tmp2 = __lsx_vldrepl_d(src_argb, 0);
    src_argb += stepx;
    tmp3 = __lsx_vldrepl_d(src_argb, 0);
    src_argb += stepx;
    tmp4 = __lsx_vldrepl_d(next_argb, 0);
    next_argb += stepx;
    tmp5 = __lsx_vldrepl_d(next_argb, 0);
    next_argb += stepx;
    tmp6 = __lsx_vldrepl_d(next_argb, 0);
    next_argb += stepx;
    tmp7 = __lsx_vldrepl_d(next_argb, 0);
    next_argb += stepx;
    DUP4_ARG2(__lsx_vilvl_d, tmp1, tmp0, tmp3, tmp2, tmp5, tmp4,
              tmp7, tmp6, src0, src1, src2, src3);
    DUP2_ARG2(__lsx_vaddwev_h_bu, src0, src2, src1, src3, tmp0, tmp2);
    DUP2_ARG2(__lsx_vaddwod_h_bu, src0, src2, src1, src3, tmp1, tmp3);
    DUP2_ARG2(__lsx_vpackev_w, tmp1, tmp0, tmp3, tmp2, reg0, reg1);
    DUP2_ARG2(__lsx_vpackod_w, tmp1, tmp0, tmp3, tmp2, tmp4, tmp5);
    DUP2_ARG2(__lsx_vadd_h, reg0, tmp4, reg1, tmp5, reg0, reg1);
    dst0 = __lsx_vsrarni_b_h(reg1, reg0, 2);
    dst0 = __lsx_vshuf4i_b(dst0, 0xD8);
    __lsx_vst(dst0, dst_argb, 0);
    dst_argb += 16;
  }
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // !defined(LIBYUV_DISABLE_LSX) && defined(__loongarch_sx)
