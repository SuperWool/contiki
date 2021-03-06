/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.util.ArrayList;

/**
 * Node model, holds the Node's address and all packets that the Node either
 * sent or received.
 *
 * @author Ed Crampin
 */
public class Node {

    private final String address;
    private final ArrayList<Packet> packets;

    /**
     * Constructor class for a node.
     *
     * @param address the address of the Node, minus first 4 hex chars.
     */
    public Node(String address) {
        this.address = address;
        this.packets = new ArrayList<>();
    }

    /**
     * Returns the packets sent/received by the node.
     *
     * @return ArrayList of all packets sent/received by the Node.
     */
    public ArrayList<Packet> getPackets() {
        return this.packets;
    }

    /**
     * Adds a new packet to the list held by the Node.
     *
     * @param packet
     */
    public void addPacket(Packet packet) {
        this.packets.add(packet);
    }

    /**
     * Returns IPv6 address of the node
     *
     * @return IPv6 address of the node
     */
    public String getAddress() {
        return this.address;
    }

    /**
     * Return raw address of node.
     *
     * @deprecated will now return getAddress()
     * @see getAddress
     */
    public String getRawAddress() {
        return this.getAddress();
    }

    /**
     * Gets the 'identifier' of the node.
     *
     * @return last 4 characters of node's IPv6 address.
     */
    public String getIdentifier() {
        return this.address.substring(this.address.length() - 4, this.address.length());
    }
}
