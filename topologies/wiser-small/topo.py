"""
Example topology of Quagga routers
"""

import inspect
import os
from mininext.topo import Topo
from mininext.services.quagga import QuaggaService
#from mininext.services.redis import RedisService

from collections import namedtuple

QuaggaHost = namedtuple("QuaggaHost", "name ip loIP")
net = None


class QuaggaTopo(Topo):

    "Creates a topology of Quagga routers"

    def __init__(self):
        """Initialize a Quagga topology with 5 routers, configure
           their IP addresses, loop back interfaces, and paths to
           their private configuration directories. Also start a redis
           sever at IP address 172.1.1.1.  Chain topology is A -> B ->
           C -> D -> E."""
        Topo.__init__(self)

        # Directory where this file / script is located"
        selfPath = os.path.dirname(os.path.abspath(
            inspect.getfile(inspect.currentframe())))  # script directory

        # Initialize a service helper for Quagga with default options
        quaggaSvc = QuaggaService(autoStop=False)
 #       redisSvc = RedisService(autoStop=False)

        # Path configurations for mounts
        quaggaBaseConfigPath = selfPath + '/configs/'
#        redisBaseConfigPath = selfPath + '/configs/'

        # List of Quagga host configs
        quaggaHosts = []
        quaggaHosts.append(QuaggaHost(name='a1', ip='172.0.1.1/16',
                                      loIP='10.0.1.1/24'))
        quaggaHosts.append(QuaggaHost(name='b1', ip='172.0.2.1/16',
                                      loIP='10.0.2.1/24'))
        quaggaHosts.append(QuaggaHost(name='c1', ip='172.0.3.1/16',
                                      loIP='10.0.3.1/24'))

        loadHosts = []

        # load generators
        # (no)
        
        # Add switch for IXP fabric
        ixpfabric = self.addSwitch('fabric-sw1')
#        testHost = self.addHost(name='test')

        # Setup each Quagga router, add a link between it and the IXP fabric
        for host in quaggaHosts:

            # Create an instance of a host, called a quaggaContainer
            quaggaContainer = self.addHost(name=host.name,
                                           ip=host.ip,
                                           hostname=host.name,
                                           privateLogDir=True,
                                           privateRunDir=True,
                                           inMountNamespace=True,
                                           inPIDNamespace=True,
                                           inUTSNamespace=True)

            # Add a loopback interface with an IP in router's announced range
            self.addNodeLoopbackIntf(node=host.name, ip=host.loIP)

            # Configure and setup the Quagga service for this node
            quaggaSvcConfig = \
                {'quaggaConfigPath': quaggaBaseConfigPath + host.name}
            self.addNodeService(node=host.name, service=quaggaSvc,
                                nodeConfig=quaggaSvcConfig)

            # Attach the quaggaContainer to the IXP Fabric Switch
            self.addLink(ixpfabric, quaggaContainer)



