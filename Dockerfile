FROM dynainstrumentsoss/ptxdist-arm-v5te:2017-09

LABEL maintainer="Johannes Lode, DYNA Instruments GmbH Hamburg, http://www.dynainstruments.com"

USER ptxdist
VOLUME /tftpboot

# add some prefetched distfiles, which are hard to get or lost on the net
ADD cached-distfiles.tar /home/ptxdist/distfiles/

# add the BSP
ADD DYNA-ECO-G45_BSP_linux-4.9.tar /home/ptxdist/

WORKDIR /home/ptxdist/DYNA-BSP
CMD ["/bin/bash", "-il"]
