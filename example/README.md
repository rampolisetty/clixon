# Clixon yang routing example

## Compile and run
```
    cd example
    make && sudo make install
```
Start backend:
```
    clixon_backend -f /usr/local/etc/routing.xml -I
```
Edit cli:
```
    clixon_cli -f /usr/local/etc/routing.xml
```
Send netconf command:
```
    clixon_netconf -f /usr/local/etc/routing.xml
```

## Setting data example using netconf
```
<rpc><edit-config><target><candidate/></target><config>
      <interfaces>
         <interface>
            <name>eth1</name>
            <enabled>true</enabled>
            <ipv4>
               <address>
                  <ip>9.2.3.4</ip>
                  <prefix-length>24</prefix-length>
               </address>
            </ipv4>
         </interface>
      </interfaces>
</config></edit-config></rpc>]]>]]>
```

## Getting data using netconf
```
<rpc><get-config><source><candidate/></source></get-config></rpc>]]>]]>
<rpc><get-config><source><candidate/></source><filter/></get-config></rpc>]]>]]>
<rpc><get-config><source><candidate/></source><filter type="xpath"/></get-config></rpc>]]>]]>
<rpc><get-config><source><candidate/></source><filter type="subtree"><configuration><interfaces><interface><ipv4/></interface></interfaces></configuration></filter></get-config></rpc>]]>]]>
<rpc><get-config><source><candidate/></source><filter type="xpath" select="/interfaces/interface/ipv4"/></get-config></rpc>]]>]]>
<rpc><validate><source><candidate/></source></validate></rpc>]]>]]>
```

## Creating notification

The example has an example notification triggering every 10s. To start a notification 
stream in the session, create a subscription:
```
<rpc><create-subscription><stream>ROUTING</stream></create-subscription></rpc>]]>]]>
<rpc-reply><ok/></rpc-reply>]]>]]>
<notification><event>Routing notification</event></notification>]]>]]>
<notification><event>Routing notification</event></notification>]]>]]>
...
```
This can also be triggered via the CLI:
```
cli> notify 
cli> Routing notification
Routing notification
...
```

## Operation data

Clixon implements Yang RPC operations by an extension mechanism. The
extension mechanism enables you to add application-specific
operations. It works by adding user-defined callbacks for added
netconf operations. It is possible to use the extension mechanism
independent of the yang rpc construct, but it is recommended. The example includes an example:

Example:
```
cli> rpc ipv4
<rpc-reply>
   <ok/>
</rpc-reply>
```

The example works by creating a netconf rpc call and sending it to the backend: (see the fib_route_rpc() function).
```
  <rpc>
    <fib-route>
      <routing-instance-name>ipv4</routing-instance-name>
    </fib-route>
   </rpc>
```

In the backend, a callback is registered (fib_route()) which handles the RPC.
```
static int 
fib_route(clicon_handle h, 
	  cxobj        *xe,           /* Request: <rpc><xn></rpc> */
	  struct client_entry *ce,    /* Client session */
	  cbuf         *cbret,        /* Reply eg <rpc-reply>... */
	  void         *arg)          /* Argument given at register */
{
    cprintf(cbret, "<rpc-reply><ok/></rpc-reply>");    
    return 0;
}
int
plugin_init(clicon_handle h)
{
...
   backend_rpc_cb_register(h, fib_route, NULL, "fib-route");
...
}
```
## State data

Netconf <get> and restconf GET also returns state data, in contrast to
config data. 

In YANG state data is specified with "config false;". In the example, interface-state is state data.

To return state data, you need to write a backend state data callback
with the name "plugin_statedata" where you return an XML tree with
state. This is then merged with config data by the system.

A static example of returning state data is in the example. Note that
a real example would poll or get the interface counters via a system
call, as well as use the "xpath" argument to identify the requested
state data.


## Run as docker container
```
cd docker
# look in README
```



