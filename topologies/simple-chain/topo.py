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
        quaggaHosts.append(QuaggaHost(name='d1', ip='172.0.4.1/16',
                                      loIP='10.0.4.1/24'))

        loadHosts = []

        # load generators
        loadHosts.append(QuaggaHost(name='t1', ip='172.0.10.1/16',
                                      loIP='10.0.10.1/24'))
        loadHosts.append(QuaggaHost(name='t2', ip='172.0.11.1/16',
                                      loIP='10.0.11.1/24'))
        loadHosts.append(QuaggaHost(name='t3', ip='172.0.12.1/16',
                                      loIP='10.0.12.1/24'))
        loadHosts.append(QuaggaHost(name='t4', ip='172.0.13.1/16',
                                      loIP='10.0.13.1/24'))
        loadHosts.append(QuaggaHost(name='t5', ip='172.0.14.1/16',
                                      loIP='10.0.14.1/24'))
        loadHosts.append(QuaggaHost(name='t6', ip='172.0.15.1/16',
                                      loIP='10.0.15.1/24'))
        loadHosts.append(QuaggaHost(name='t7', ip='172.0.16.1/16',
                                      loIP='10.0.16.1/24'))
        loadHosts.append(QuaggaHost(name='t8', ip='172.0.17.1/16',
                                      loIP='10.0.17.1/24'))


        # Add switch for IXP fabric
        ixpfabric = self.addSwitch('fabric-sw1')
#        testHost = self.addHost(name='test')
        loadfabric = self.addSwitch('fabric-sw2')
        self.addLink(ixpfabric, loadfabric)


        

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

        for host in loadHosts:

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
            self.addLink(loadfabric, quaggaContainer)

#        self.addLink(ixpfabric, testHost)


