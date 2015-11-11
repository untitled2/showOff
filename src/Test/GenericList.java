/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Test;

import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 *
 * @author nikitaishkov
 * @param <E>
 */
public class GenericList<E> implements Iterable<E>, Iterator<E>
{
    private Node head, tail, next, lastReturned;
    public long size, count;
    
    public GenericList() {
        head = null;
        size = 0;
    }
    
    public void add(E data) {
        if(size == 0) {
            head = tail = new Node(data, null);
        }
        else {
            Node curr = new Node(data, tail);
            tail.next = curr;
            tail = curr;
        }
        
        size++;
    }
    
    public E head() {
        return head.getData();
    }

    @Override
    public Iterator<E> iterator() {
        count = 0;
        next = head;
        return this;
    }

    @Override
    public boolean hasNext() {
        return count < size;
    }

    @Override
    public E next() {
        if (!hasNext())
            throw new NoSuchElementException();

        lastReturned = next;
        next = next.next;
        count++;
        
        return lastReturned.data;
    }
    
    private class Node
    {
        final E data;
        Node prev, next;
        
        private Node(E element, Node prev) {
            data = element;
            this.prev = prev;
        }
        
        private E getData() {
            return data;
        }
    }
}
