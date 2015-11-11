/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Test;

/**
 *
 * @author nikitaishkov
 */
public class Test {
    public static void main(String[] args) {
        GenericList<String> strList = new GenericList<>();
        strList.add("Hello,");
        strList.add(" ");
        strList.add("world");
        strList.add("!");
        
        System.out.println(strList.size);
        
        for(String s : strList)
            System.out.print(s);
        System.out.println();
        
        for(String s : strList)
            System.out.print(s);
        System.out.println();
    }
}
