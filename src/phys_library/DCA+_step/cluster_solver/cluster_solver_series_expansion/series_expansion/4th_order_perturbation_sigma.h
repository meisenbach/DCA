//-*-C++-*-

#ifndef COMPUTE_FOURTH_ORDER_SIGMA_H
#define COMPUTE_FOURTH_ORDER_SIGMA_H
#include"phys_library/domain_types.hpp"
using namespace types;

namespace DCA
{

  namespace SERIES_EXPANSION
  {

    /*!
     * \authors Peter Staar, Urs R. Hahener
     *
     * \brief  This class implements the computation of the self-energy in second order.
     */
    template<class parameter_type, class k_dmn_t>
    class sigma_perturbation<4, parameter_type, k_dmn_t>
    {

      typedef typename parameter_type::profiler_type    profiler_t;
      typedef typename parameter_type::concurrency_type concurrency_t;

    public:

      typedef typename compute_interaction::function_type U_type;

      typedef compute_bubble<ph, parameter_type, k_dmn_t, w> ph_bubble_t;
      typedef compute_bubble<ph, parameter_type, k_dmn_t, w> pp_bubble_t;

      typedef typename ph_bubble_t::function_type chi_type;
      typedef typename pp_bubble_t::function_type phi_type;

      typedef FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > function_type;

    public:

      sigma_perturbation(parameter_type&      parameters_ref,
                         compute_interaction& interaction_obj,
                         compute_bubble<ph, parameter_type, k_dmn_t, w>& chi_obj,
                         compute_bubble<pp, parameter_type, k_dmn_t, w>& phi_obj);

      ~sigma_perturbation();

      function_type& get_function() { return Sigma; }

      template<class stream_type>
      void to_JSON(stream_type& ss);

      void execute_on_cluster(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                              FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2);

      template<IO::FORMAT DATA_FORMAT>
      void write(IO::writer<DATA_FORMAT>& writer);

    private:

      void execute_4A(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4C(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4D(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2);
      void execute_4E(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2);
      void execute_4F(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2);
      void execute_4G(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4H(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4J_old(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4J(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4K(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_4L(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);


      int subtract_freq_fb(int, int); // boson-fermion
      int subtract_freq_bf(int, int); // fermion-boson
      int subtract_freq_ff(int, int); // fermion-fermion
      int add_freq_ff(int, int);      // fermion+fermion
      int add_freq_bf(int, int);      // boson+fermion

    protected:

      parameter_type& parameters;
      concurrency_t& concurrency;

      U_type& U;

      chi_type& chi;
      phi_type& phi;

      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4A;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4C;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4D;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4E;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4F;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4G;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4H;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4J;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4K;
      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > Sigma_4L;
    };

    template<class parameter_type, class k_dmn_t>
    sigma_perturbation<4, parameter_type, k_dmn_t>::sigma_perturbation(parameter_type&      parameters_ref,
                                                                       compute_interaction& interaction_obj,
                                                                       compute_bubble<ph, parameter_type, k_dmn_t, w>& chi_obj,
                                                                       compute_bubble<pp, parameter_type, k_dmn_t, w>& phi_obj):
      parameters(parameters_ref),
      concurrency(parameters.get_concurrency()),

      U(interaction_obj.get_function()),

      chi(chi_obj.get_function()),
      phi(phi_obj.get_function()),

      Sigma("Sigma-4th-order"),
      Sigma_4A("Sigma-4th-order-A"),
      Sigma_4C("Sigma-4th-order-C"),
      Sigma_4D("Sigma-4th-order-D"),
      Sigma_4E("Sigma-4th-order-E"),
      Sigma_4F("Sigma-4th-order-F"),
      Sigma_4G("Sigma-4th-order-G"),
      Sigma_4H("Sigma-4th-order-H"),
      Sigma_4J("Sigma-4th-order-J"),
      Sigma_4K("Sigma-4th-order-K"),
      Sigma_4L("Sigma-4th-order-L")
    {}

    template<class parameter_type, class k_dmn_t>
    sigma_perturbation<4, parameter_type, k_dmn_t>::~sigma_perturbation()
    {}
    
    template<class parameter_type, class k_dmn_t>
    template<IO::FORMAT DATA_FORMAT>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::write(IO::writer<DATA_FORMAT>& /*writer*/)
    {

    }

//     template<class parameter_type, class k_dmn_t>
//     template<class stream_type>
//     void sigma_perturbation<4, parameter_type, k_dmn_t>::to_JSON(stream_type& ss)
//     {
//       Sigma.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4A.to_JSON(ss);
//       ss << ",\n";

//       //Sigma_4B.to_JSON(ss);
//       //ss << ",\n";

//       Sigma_4C.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4D.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4E.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4F.to_JSON(ss);
//       ss << "\n";

//       Sigma_4G.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4H.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4J.to_JSON(ss);
//       ss << ",\n";

//       Sigma_4K.to_JSON(ss);
//       ss << "\n";

//       Sigma_4L.to_JSON(ss);
//       ///ss << ",\n";
//     }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_on_cluster(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                                                                            FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2)
    {
      std::cout << __FUNCTION__ << std::endl;

      std::cout << "\t U : " << U(0,0,0,1) << std::endl;

      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4A(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4C(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4D(G, Sigma_2);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4E(G, Sigma_2);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4F(G, Sigma_2);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4G(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4H(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4J(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4K(G);
      sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4L(G);

      Sigma = 0.;
      Sigma += Sigma_4A;
      Sigma += Sigma_4A; // Sigma_4B = Sigma_4A
      Sigma += Sigma_4C;
      Sigma += Sigma_4D;
      Sigma += Sigma_4E;
      Sigma += Sigma_4F;
      Sigma += Sigma_4G;
      Sigma += Sigma_4H;
      Sigma += Sigma_4H; // Sigma_4I = Sigma_4H
      Sigma += Sigma_4J;
      Sigma += Sigma_4K;
      Sigma += Sigma_4L;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4A(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4A = 0.;

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
        for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

          int nu_c = (nu_ind-w_VERTEX_BOSONIC::dmn_size()/2);

          for(int w_ind=std::fabs(nu_c); w_ind<w::dmn_size()-std::fabs(nu_c); ++w_ind){
            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){

              int k_minus_q = k_dmn_t::parameter_type::subtract(q_ind, k_ind);
              int w_minus_nu = w_ind-nu_c;

              Sigma_4A(0,0, 0,0, k_ind, w_ind) += G(0,0, 0,0, k_minus_q, w_minus_nu) * chi(0,0, 0,0, q_ind, nu_ind)
                * chi(0,0, 0,0, q_ind, nu_ind) * chi(0,0, 0,0, q_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4A(0,1, 0,1, k_ind, w_ind) = Sigma_4A(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value*U_value*U_value;
      Sigma_4A *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4C(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4C = 0.;

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
        for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int nu_minus_w = subtract_freq_fb(w_ind, nu_ind);
            if (nu_minus_w<0 || nu_minus_w >= w::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int q_minus_k = k_dmn_t::parameter_type::subtract(k_ind, q_ind);

              Sigma_4C(0,0, 0,0, k_ind, w_ind) += G(0,0, 0,0, q_minus_k, nu_minus_w) * phi(0,0, 0,0, q_ind, nu_ind)
                * phi(0,0, 0,0, q_ind, nu_ind) * phi(0,0, 0,0, q_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4C(0,1, 0,1, k_ind, w_ind) = Sigma_4C(0,0, 0,0, k_ind, w_ind);

      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value*U_value*U_value;
      Sigma_4C *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4D(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                                                                    FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4D = 0.;

      for(int nu_ind=0; nu_ind<w::dmn_size(); ++nu_ind){
        for(int p_ind=0; p_ind<k_dmn_t::dmn_size(); ++p_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int w_minus_nu = subtract_freq_ff(nu_ind, w_ind);
            if (w_minus_nu<0 || w_minus_nu >= w_VERTEX_BOSONIC::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int k_minus_p = k_dmn_t::parameter_type::subtract(p_ind, k_ind);

              Sigma_4D(0,0, 0,0, k_ind, w_ind) += chi(0,0, 0,0, k_minus_p,w_minus_nu) * G(0,0, 0,0, p_ind, nu_ind)
                * G(0,0, 0,0, p_ind, nu_ind) * Sigma_2(0,0, 0,0, p_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4D(0,1, 0,1, k_ind, w_ind) = Sigma_4D(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value;
      Sigma_4D *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4E(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                                                                    FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4E = 0.;

      for(int nu_ind=0; nu_ind<w::dmn_size(); ++nu_ind){
        for(int p_ind=0; p_ind<k_dmn_t::dmn_size(); ++p_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int nu_minus_w = subtract_freq_ff(w_ind, nu_ind);
            if (nu_minus_w<0 || nu_minus_w >= w_VERTEX_BOSONIC::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int p_minus_k = k_dmn_t::parameter_type::subtract(k_ind, p_ind);

              Sigma_4E(0,0, 0,0, k_ind, w_ind) += chi(0,0, 0,0, p_minus_k,nu_minus_w) * G(0,0, 0,0, p_ind, nu_ind)
                * G(0,0, 0,0, p_ind, nu_ind) * Sigma_2(0,0, 0,0, p_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4E(0,1, 0,1, k_ind, w_ind) = Sigma_4E(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value;
      Sigma_4E *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4F(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G,
                                                                    FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& Sigma_2)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4F = 0.;

      for(int nu_ind=0; nu_ind<w::dmn_size(); ++nu_ind){
        for(int p_ind=0; p_ind<k_dmn_t::dmn_size(); ++p_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int w_plus_nu = add_freq_ff(w_ind, nu_ind);
            if (w_plus_nu<0 || w_plus_nu >= w_VERTEX_BOSONIC::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int k_plus_p = k_dmn_t::parameter_type::add(k_ind, p_ind);

              Sigma_4F(0,0, 0,0, k_ind, w_ind) += phi(0,0, 0,0, k_plus_p, w_plus_nu) * G(0,0, 0,0, p_ind, nu_ind)
                * G(0,0, 0,0, p_ind, nu_ind) * Sigma_2(0,0, 0,0, p_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4F(0,1, 0,1, k_ind, w_ind) = Sigma_4F(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value;
      Sigma_4F *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4G(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4G = 0.;

      dmn_2<w_VERTEX_BOSONIC, k_dmn_t> dmn;
      dca::concurrency::thread_manager_sum<concurrency_t> sum_manager(concurrency);

      do
        {
          std::pair<int, int> bounds = sum_manager.get_bounds(dmn);

          int* coordinates = new int[2];

          for(int l=bounds.first; l<bounds.second; l++)
            {
              dmn.linind_2_subind(l, coordinates);

              int nu_ind1 = coordinates[0];
              int q_ind1  = coordinates[1];

              //for(int nu_ind1=0; nu_ind1<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind1){
              //for(int q_ind1=0; q_ind1<k_dmn_t::dmn_size(); ++q_ind1){

              for(int nu_ind2=0; nu_ind2<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind2){
                for(int q_ind2=0; q_ind2<k_dmn_t::dmn_size(); ++q_ind2){

                  for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
                    int w_minus_nu1 = subtract_freq_bf(nu_ind1, w_ind);
                    int w_minus_nu2 = subtract_freq_bf(nu_ind2, w_ind);
                    int w_minus_nu1_minus_nu2 = subtract_freq_bf(nu_ind2, w_minus_nu1);
                    if (w_minus_nu1 < 0 || w_minus_nu1 >= w::dmn_size() || w_minus_nu2 < 0 || w_minus_nu2 >= w::dmn_size()
                        || w_minus_nu1_minus_nu2 < 0 || w_minus_nu1_minus_nu2 >= w::dmn_size()) continue;

                    for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
                      int k_minus_q1 = k_dmn_t::parameter_type::subtract(q_ind1, k_ind);
                      int k_minus_q2 = k_dmn_t::parameter_type::subtract(q_ind2, k_ind);
                      int k_minus_q1_minus_q2 = k_dmn_t::parameter_type::subtract(q_ind2, k_minus_q1);

                      Sigma_4G(0,0, 0,0, k_ind, w_ind) += chi(0,0, 0,0, q_ind1, nu_ind1) * chi(0,0, 0,0, q_ind2, nu_ind2)
                        * G(0,0, 0,0, k_minus_q1, w_minus_nu1) * G(0,0, 0,0, k_minus_q2, w_minus_nu2)
                        * G(0,0, 0,0, k_minus_q1_minus_q2, w_minus_nu1_minus_nu2);

                    }
                  }
                }
              }
            }
          //}
          delete [] coordinates;
        }
      while(!sum_manager.sum_and_check(Sigma_4G));

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4G(0,1, 0,1, k_ind, w_ind) = Sigma_4G(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*parameters.get_beta() *k_dmn_t::dmn_size()* k_dmn_t::dmn_size())
        *U_value*U_value*U_value*U_value;
      Sigma_4G *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4H(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4H = 0.;

      dmn_2<w_VERTEX_BOSONIC, k_dmn_t> dmn;
      dca::concurrency::thread_manager_sum<concurrency_t> sum_manager(concurrency);

      do
        {
          std::pair<int, int> bounds = sum_manager.get_bounds(dmn);

          int* coordinates = new int[2];

          for(int l=bounds.first; l<bounds.second; l++)
            {
              dmn.linind_2_subind(l, coordinates);

              int nu_ind1 = coordinates[0];
              int q_ind1  = coordinates[1];

              //for(int nu_ind1=0; nu_ind1<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind1){
              //for(int q_ind1=0; q_ind1<k_dmn_t::dmn_size(); ++q_ind1){

              for(int nu_ind2=0; nu_ind2<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind2){
                for(int q_ind2=0; q_ind2<k_dmn_t::dmn_size(); ++q_ind2){

                  for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
                    int w_minus_nu1 = subtract_freq_bf(nu_ind1, w_ind);
                    int nu2_minus_w = subtract_freq_fb(w_ind, nu_ind2);
                    int nu1_plus_nu2_minus_w = add_freq_bf(nu_ind1, nu2_minus_w);
                    if (w_minus_nu1 < 0 || w_minus_nu1 >= w::dmn_size() || nu2_minus_w < 0 || nu2_minus_w >= w::dmn_size()
                        || nu1_plus_nu2_minus_w < 0 || nu1_plus_nu2_minus_w >= w::dmn_size()) continue;

                    for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
                      int k_minus_q1 = k_dmn_t::parameter_type::subtract(q_ind1, k_ind);
                      int q2_minus_k = k_dmn_t::parameter_type::subtract(k_ind, q_ind2);
                      int q1_plus_q2_minus_k = k_dmn_t::parameter_type::add(q_ind1, q2_minus_k);

                      Sigma_4H(0,0, 0,0, k_ind, w_ind) += chi(0,0, 0,0, q_ind1, nu_ind1) * phi(0,0, 0,0, q_ind2, nu_ind2)
                        * G(0,0, 0,0, k_minus_q1, w_minus_nu1) * G(0,0, 0,0, q2_minus_k, nu2_minus_w)
                        * G(0,0, 0,0, q1_plus_q2_minus_k, nu1_plus_nu2_minus_w);
                    }
                  }
                }
              }
            }
          //}
          delete [] coordinates;
        }
      while(!sum_manager.sum_and_check(Sigma_4H));

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4H(0,1, 0,1, k_ind, w_ind) = Sigma_4H(0,0, 0,0, k_ind, w_ind);


      double factor = -1./(parameters.get_beta()*parameters.get_beta() *k_dmn_t::dmn_size()*k_dmn_t::dmn_size())
        *U_value*U_value*U_value*U_value;
      Sigma_4H *= factor;
    }

    /*
      template<class parameter_type, class k_dmn_t>
      void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4J_old(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
      {
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4J = 0.;

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
      for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

      for(int w1_ind=0; w1_ind<w::dmn_size(); ++w1_ind){
      int nu_minus_w1 = subtract_freq_fb(w1_ind, nu_ind);
      if(nu_minus_w1 < 0 || nu_minus_w1 >= w::dmn_size()) continue;
      for(int p1_ind=0; p1_ind<k_dmn_t::dmn_size(); ++p1_ind){

      for(int w2_ind=0; w2_ind<w::dmn_size(); ++w2_ind){
      int nu_minus_w2 = subtract_freq_fb(w2_ind, nu_ind);
      if(nu_minus_w2 < 0 || nu_minus_w2 >= w::dmn_size()) continue;
      int nu_minus_w1_minus_w2 = subtract_freq_ff(w2_ind, nu_minus_w1);
      if(nu_minus_w1_minus_w2 < 0 || nu_minus_w1_minus_w2 >= w_VERTEX_BOSONIC::dmn_size()) continue;
      for(int p2_ind=0; p2_ind<k_dmn_t::dmn_size(); ++p2_ind){

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
      int nu_minus_w = subtract_freq_fb(w_ind, nu_ind);
      if(nu_minus_w < 0 || nu_minus_w >= w::dmn_size()) continue;

      for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
      int q_minus_k = k_dmn_t::parameter_type::subtract(k_ind, q_ind);
      int q_minus_p1 = k_dmn_t::parameter_type::subtract(p1_ind, q_ind);
      int q_minus_p2 = k_dmn_t::parameter_type::subtract(p2_ind, q_ind);
      int q_minus_p1_minus_p2 = k_dmn_t::parameter_type::subtract(p2_ind, q_minus_p1);

      Sigma_4J(0,0, 0,0, k_ind, w_ind) += chi(0,0, 0,0, q_minus_p1_minus_p2, nu_minus_w1_minus_w2)
      * G(0,0, 0,0, p1_ind, w1_ind) * G(0,0, 0,0, p2_ind, w2_ind) * G(0,0, 0,0, q_minus_p1, nu_minus_w1)
      * G(0,0, 0,0, q_minus_p2, nu_minus_w2) * G(0,0, 0,0, q_minus_k, nu_minus_w);
      }
      }
      }
      }
      }
      }
      }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
      for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
      Sigma_4J(0,1, 0,1, k_ind, w_ind) = Sigma_4J(0,0, 0,0, k_ind, w_ind);


      double factor = 1./(parameters.get_beta()*parameters.get_beta()*parameters.get_beta()
      *k_dmn_t::dmn_size()*k_dmn_t::dmn_size()*k_dmn_t::dmn_size())
      *U_value*U_value*U_value*U_value;
      Sigma_4J *= factor;
      }
    */

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4J(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4J = 0.;

      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w_VERTEX_BOSONIC> > F; // Calculating part independent of k

      dmn_2<w_VERTEX_BOSONIC, k_dmn_t> dmn;
      dca::concurrency::thread_manager_sum<concurrency_t> sum_manager(concurrency);

      do
        {
          std::pair<int, int> bounds = sum_manager.get_bounds(dmn);

          int* coordinates = new int[2];

          for(int l=bounds.first; l<bounds.second; l++)
            {
              dmn.linind_2_subind(l, coordinates);

              int nu_ind = coordinates[0];
              int q_ind  = coordinates[1];

              //for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
              //for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

              for(int w1_ind=0; w1_ind<w::dmn_size(); ++w1_ind){
                int nu_minus_w1 = subtract_freq_fb(w1_ind, nu_ind);
                if(nu_minus_w1 < 0 || nu_minus_w1 >= w::dmn_size()) continue;
                for(int p1_ind=0; p1_ind<k_dmn_t::dmn_size(); ++p1_ind){

                  for(int w2_ind=0; w2_ind<w::dmn_size(); ++w2_ind){
                    int nu_minus_w2 = subtract_freq_fb(w2_ind, nu_ind);
                    if(nu_minus_w2 < 0 || nu_minus_w2 >= w::dmn_size()) continue;
                    int nu_minus_w1_minus_w2 = subtract_freq_ff(w2_ind, nu_minus_w1);
                    if(nu_minus_w1_minus_w2 < 0 || nu_minus_w1_minus_w2 >= w_VERTEX_BOSONIC::dmn_size()) continue;
                    for(int p2_ind=0; p2_ind<k_dmn_t::dmn_size(); ++p2_ind){

                      int q_minus_p1 = k_dmn_t::parameter_type::subtract(p1_ind, q_ind);
                      int q_minus_p2 = k_dmn_t::parameter_type::subtract(p2_ind, q_ind);
                      int q_minus_p1_minus_p2 = k_dmn_t::parameter_type::subtract(p2_ind, q_minus_p1);

                      F(0,0, 0,0, q_ind, nu_ind) += chi(0,0, 0,0, q_minus_p1_minus_p2, nu_minus_w1_minus_w2)
                        * G(0,0, 0,0, p1_ind, w1_ind) * G(0,0, 0,0, p2_ind, w2_ind) * G(0,0, 0,0, q_minus_p1, nu_minus_w1)
                        * G(0,0, 0,0, q_minus_p2, nu_minus_w2);
                    }
                  }
                }
              }
            }

          delete [] coordinates;
        }
      while(!sum_manager.sum_and_check(F));

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
        for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int nu_minus_w = subtract_freq_fb(w_ind, nu_ind);
            if(nu_minus_w < 0 || nu_minus_w >= w::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int q_minus_k = k_dmn_t::parameter_type::subtract(k_ind, q_ind);

              Sigma_4J(0,0, 0,0, k_ind, w_ind) += F(0,0, 0,0, q_ind, nu_ind) * G(0,0, 0,0, q_minus_k, nu_minus_w);
            }
          }
        }
      }


      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4J(0,1, 0,1, k_ind, w_ind) = Sigma_4J(0,0, 0,0, k_ind, w_ind);

      double factor = 1./(parameters.get_beta()*parameters.get_beta()*parameters.get_beta()
                          *k_dmn_t::dmn_size()*k_dmn_t::dmn_size()*k_dmn_t::dmn_size())
        *U_value*U_value*U_value*U_value;
      Sigma_4J *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4K(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4K = 0.;

      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w_VERTEX_BOSONIC> > F; // Calculating part independent of k

      dmn_2<w_VERTEX_BOSONIC, k_dmn_t> dmn;
      dca::concurrency::thread_manager_sum<concurrency_t> sum_manager(concurrency);

      do
        {
          std::pair<int, int> bounds = sum_manager.get_bounds(dmn);

          int* coordinates = new int[2];

          for(int l=bounds.first; l<bounds.second; l++)
            {
              dmn.linind_2_subind(l, coordinates);

              int nu_ind = coordinates[0];
              int q_ind  = coordinates[1];

              //     for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
              //       for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

              for(int w1_ind=0; w1_ind<w::dmn_size(); ++w1_ind){
                int nu_plus_w1 = add_freq_bf(nu_ind, w1_ind);
                if(nu_plus_w1 < 0 || nu_plus_w1 >= w::dmn_size()) continue;
                for(int p1_ind=0; p1_ind<k_dmn_t::dmn_size(); ++p1_ind){

                  for(int w2_ind=0; w2_ind<w::dmn_size(); ++w2_ind){
                    int nu_plus_w2 = add_freq_bf(nu_ind, w2_ind);
                    if(nu_plus_w2 < 0 || nu_plus_w2 >= w::dmn_size()) continue;
                    int nu_plus_w1_plus_w2 = add_freq_ff(nu_plus_w1, w2_ind);
                    if(nu_plus_w1_plus_w2 < 0 || nu_plus_w1_plus_w2 >= w_VERTEX_BOSONIC::dmn_size()) continue;
                    for(int p2_ind=0; p2_ind<k_dmn_t::dmn_size(); ++p2_ind){

                      int q_plus_p1 = k_dmn_t::parameter_type::add(q_ind, p1_ind);
                      int q_plus_p2 = k_dmn_t::parameter_type::add(q_ind, p2_ind);
                      int q_plus_p1_plus_p2 = k_dmn_t::parameter_type::add(q_plus_p1, p2_ind);

                      F(0,0, 0,0, q_ind, nu_ind) += phi(0,0, 0,0, q_plus_p1_plus_p2, nu_plus_w1_plus_w2)
                        * G(0,0, 0,0, p1_ind, w1_ind) * G(0,0, 0,0, p2_ind, w2_ind) * G(0,0, 0,0, q_plus_p1, nu_plus_w1)
                        * G(0,0, 0,0, q_plus_p2, nu_plus_w2);
                    }
                  }
                }
              }
            }

          delete [] coordinates;
        }
      while(!sum_manager.sum_and_check(F));

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
        for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int w_minus_nu = subtract_freq_bf(nu_ind, w_ind);
            if(w_minus_nu < 0 || w_minus_nu >= w::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int k_minus_q = k_dmn_t::parameter_type::subtract(q_ind, k_ind);

              Sigma_4K(0,0, 0,0, k_ind, w_ind) += F(0,0,0,0,q_ind, nu_ind) * G(0,0, 0,0, k_minus_q, w_minus_nu);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4K(0,1, 0,1, k_ind, w_ind) = Sigma_4K(0,0, 0,0, k_ind, w_ind);

      double factor = 1./(parameters.get_beta()*parameters.get_beta()*parameters.get_beta()
                          *k_dmn_t::dmn_size()*k_dmn_t::dmn_size()*k_dmn_t::dmn_size())
        *U_value*U_value*U_value*U_value;
      Sigma_4K *= factor;
    }

    template<class parameter_type, class k_dmn_t>
    void sigma_perturbation<4, parameter_type, k_dmn_t>::execute_4L(FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      profiler_t prof(__FUNCTION__, "SERIES EXPANSION", __LINE__);
      std::cout << __FUNCTION__ << std::endl;

      double U_value = U(0,0,0,1);

      Sigma_4L = 0.;

      FUNC_LIB::function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w_VERTEX_BOSONIC> > F; // Calculating part independent of k

      dmn_2<w_VERTEX_BOSONIC, k_dmn_t> dmn;
      dca::concurrency::thread_manager_sum<concurrency_t> sum_manager(concurrency);

      do
        {
          std::pair<int, int> bounds = sum_manager.get_bounds(dmn);

          int* coordinates = new int[2];

          for(int l=bounds.first; l<bounds.second; l++)
            {
              dmn.linind_2_subind(l, coordinates);

              int nu_ind = coordinates[0];
              int q_ind  = coordinates[1];

              //for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
              //for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

              for(int w1_ind=0; w1_ind<w::dmn_size(); ++w1_ind){
                int nu_plus_w1 = add_freq_bf(nu_ind, w1_ind);
                if(nu_plus_w1 < 0 || nu_plus_w1 >= w::dmn_size()) continue;
                for(int p1_ind=0; p1_ind<k_dmn_t::dmn_size(); ++p1_ind){

                  for(int w2_ind=0; w2_ind<w::dmn_size(); ++w2_ind){
                    int nu_plus_w2 = add_freq_bf(nu_ind, w2_ind);
                    if(nu_plus_w2 < 0 || nu_plus_w2 >= w::dmn_size()) continue;
                    int w1_minus_w2 = subtract_freq_ff(w2_ind, w1_ind);
                    if(w1_minus_w2 < 0 || w1_minus_w2 >= w_VERTEX_BOSONIC::dmn_size()) continue;
                    for(int p2_ind=0; p2_ind<k_dmn_t::dmn_size(); ++p2_ind){

                      int q_plus_p1 = k_dmn_t::parameter_type::add(q_ind, p1_ind);
                      int q_plus_p2 = k_dmn_t::parameter_type::add(q_ind, p2_ind);
                      int p1_minus_p2 = k_dmn_t::parameter_type::subtract(p2_ind, p1_ind);

                      F(0,0, 0,0, q_ind, nu_ind) += chi(0,0, 0,0, p1_minus_p2, w1_minus_w2)
                        * G(0,0, 0,0, p1_ind, w1_ind) * G(0,0, 0,0, p2_ind, w2_ind) * G(0,0, 0,0, q_plus_p1, nu_plus_w1)
                        * G(0,0, 0,0, q_plus_p2, nu_plus_w2);

                    }
                  }
                }
              }
            }
          //}
          delete [] coordinates;
        }
      while(!sum_manager.sum_and_check(F));

      for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
        for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){

          for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){
            int w_minus_nu = subtract_freq_bf(nu_ind, w_ind);
            if(w_minus_nu < 0 || w_minus_nu >= w::dmn_size()) continue;

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
              int k_minus_q = k_dmn_t::parameter_type::subtract(q_ind, k_ind);

              Sigma_4L(0,0, 0,0, k_ind, w_ind) += F(0,0,0,0,q_ind, nu_ind) * G(0,0, 0,0, k_minus_q, w_minus_nu);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_4L(0,1, 0,1, k_ind, w_ind) = Sigma_4L(0,0, 0,0, k_ind, w_ind);

      double factor = -1./(parameters.get_beta()*parameters.get_beta()*parameters.get_beta()
                           *k_dmn_t::dmn_size()*k_dmn_t::dmn_size()*k_dmn_t::dmn_size())*U_value*U_value*U_value*U_value;

      Sigma_4L *= factor;
    }


    template<class parameter_type, class k_dmn_t>
    int sigma_perturbation<4, parameter_type, k_dmn_t>::subtract_freq_fb(int w1, int w2)
    {
      int w_f = 2*(w1 - w::dmn_size()/2) + 1;             // transform fermionic
      int w_b = 2*(w2 - w_VERTEX_BOSONIC::dmn_size()/2);  // transform bosonic
      int res = ((w_b-w_f) - 1 + w::dmn_size()) / 2;      // result is fermionic
      return res;
    }

    template<class parameter_type, class k_dmn_t>
    int sigma_perturbation<4, parameter_type, k_dmn_t>::subtract_freq_bf(int w1, int w2)
    {
      int w_f = 2*(w2 - w::dmn_size()/2) + 1;             // transform fermionic
      int w_b = 2*(w1 - w_VERTEX_BOSONIC::dmn_size()/2);  // transform bosonic
      int res = ((w_f-w_b) - 1 + w::dmn_size()) / 2;      // result is fermionic
      return res;
    }

    template<class parameter_type, class k_dmn_t>
    int sigma_perturbation<4, parameter_type, k_dmn_t>::subtract_freq_ff(int w1, int w2)
    {
      int w_f1 = 2*(w1 - w::dmn_size()/2) + 1;                    // transform fermionic
      int w_f2 = 2*(w2 - w::dmn_size()/2) + 1;                    // transform fermionic
      int res = ((w_f2-w_f1) + w_VERTEX_BOSONIC::dmn_size()) / 2; // result is bosonic
      return res;
    }

    template<class parameter_type, class k_dmn_t>
    int sigma_perturbation<4, parameter_type, k_dmn_t>::add_freq_ff(int w1, int w2)
    {
      int w_f1 = 2*(w1 - w::dmn_size()/2) + 1;                    // transform fermionic
      int w_f2 = 2*(w2 - w::dmn_size()/2) + 1;                    // transform fermionic
      int res = ((w_f1+w_f2) + w_VERTEX_BOSONIC::dmn_size()) / 2; // result is bosonic
      return res;
    }

    template<class parameter_type, class k_dmn_t>
    int sigma_perturbation<4, parameter_type, k_dmn_t>::add_freq_bf(int w1, int w2)
    {
      int w_b = 2*(w1 - w_VERTEX_BOSONIC::dmn_size()/2);        // transform bosonic
      int w_f = 2*(w2 - w::dmn_size()/2) + 1;                   // transform fermionic
      int res = ((w_b+w_f) - 1 + w::dmn_size()) / 2;           // result is fermionic
      return res;
    }

  }

}

#endif
