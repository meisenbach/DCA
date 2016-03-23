//-*-C++-*-

/*
 *      Author: Peter Staar
 */


#ifndef Oh_H_
#define Oh_H_

class Oh : public point_group<3>
{
public:

  const static int size = 9;

  typedef P_3D inversion_type;

  typedef Cn_3D<1,0,0,1,4> Cn_3D_X_1_4_type;
  typedef Cn_3D<1,0,0,2,4> Cn_3D_X_2_4_type;
  typedef Cn_3D<1,0,0,3,4> Cn_3D_X_3_4_type;

  typedef Cn_3D<0,1,0,1,4> Cn_3D_Y_1_4_type;
  typedef Cn_3D<0,1,0,2,4> Cn_3D_Y_2_4_type;
  typedef Cn_3D<0,1,0,3,4> Cn_3D_Y_3_4_type;

  typedef Cn_3D<0,0,1,1,4> Cn_3D_Z_1_4_type;
  typedef Cn_3D<0,0,1,2,4> Cn_3D_Z_2_4_type;
  typedef Cn_3D<0,0,1,3,4> Cn_3D_Z_3_4_type;

  //diagonal
  typedef Cn_3D<1,1,1,1,3> Cn_3D_XYZ_1_3_type;
    typedef Cn_3D<1,1,1,2,3> Cn_3D_XYZ_2_3_type;

  typedef Sn_3D<0,0,8> Sn_3D_XY_0_8_type;
  typedef Sn_3D<0,1,8> Sn_3D_XY_1_8_type;
  typedef Sn_3D<0,2,8> Sn_3D_XY_2_8_type;
  typedef Sn_3D<0,3,8> Sn_3D_XY_3_8_type;
  typedef Sn_3D<0,4,8> Sn_3D_XY_4_8_type;

  typedef Sn_3D<1,0,8> Sn_3D_ZX_0_8_type;
  typedef Sn_3D<1,1,8> Sn_3D_ZX_1_8_type;
  typedef Sn_3D<1,2,8> Sn_3D_ZX_2_8_type;
  typedef Sn_3D<1,3,8> Sn_3D_ZX_3_8_type;
  typedef Sn_3D<1,4,8> Sn_3D_ZX_4_8_type;

  typedef Sn_3D<2,0,8> Sn_3D_YZ_0_8_type;
  typedef Sn_3D<2,1,8> Sn_3D_YZ_1_8_type;
  typedef Sn_3D<2,2,8> Sn_3D_YZ_2_8_type;
  typedef Sn_3D<2,3,8> Sn_3D_YZ_3_8_type;
  typedef Sn_3D<2,4,8> Sn_3D_YZ_4_8_type;

  typedef Typelist<inversion_type,
		     
		      Cn_3D_X_1_4_type,
		      Cn_3D_X_2_4_type,
		      Cn_3D_X_3_4_type,
		      
		      Cn_3D_Y_1_4_type,
		      Cn_3D_Y_2_4_type,
		      Cn_3D_Y_3_4_type,
		      
		      Cn_3D_Z_1_4_type,
		      Cn_3D_Z_2_4_type,
		      Cn_3D_Z_3_4_type,

		      Cn_3D_XYZ_1_3_type,
		      Cn_3D_XYZ_2_3_type,
		      
		      Sn_3D_XY_0_8_type,
		      Sn_3D_XY_1_8_type,
		      Sn_3D_XY_2_8_type,
		      Sn_3D_XY_3_8_type,
		      Sn_3D_XY_4_8_type,
		      
		      Sn_3D_ZX_0_8_type,
		      Sn_3D_ZX_1_8_type,
		      Sn_3D_ZX_2_8_type,
		      Sn_3D_ZX_3_8_type,
		      Sn_3D_ZX_4_8_type,
		      
		      Sn_3D_YZ_0_8_type,
		      Sn_3D_YZ_1_8_type,
		      Sn_3D_YZ_2_8_type,
		      Sn_3D_YZ_3_8_type,
		      Sn_3D_YZ_4_8_type> point_group_type_list;//symmetry_list;
};


#endif
