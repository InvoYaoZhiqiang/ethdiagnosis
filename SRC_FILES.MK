
SRCDIR += eth_diagnosis/src/DoIP  \
          eth_diagnosis/src/SoAd \
          eth_diagnosis/src/TcpIp_SoAd

INCDIR = eth_diagnosis/include



SRCS_$(IPU_PRIMARY_CORE) +=DoIP.c\
                           SoAd.c \
                           TcpIp_SoAd.c



SRCS_$(IPU_SECONDARY_CORE) += DoIP.c\
                              SoAd.c \
                              TcpIp_SoAd.c



