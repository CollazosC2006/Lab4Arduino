# -*- coding: utf-8 -*-
"""
UNIVERSIDAD AUTÓNOMA DEL ESTADO DE MÉXICO
CU UAEM ZUMPANGO 
UA: Sistemas Expertos
TEMA:
Alumno: Jessica Vargas Sánchez 
Profesor: Mtra. Edith Cristina Herrena Luna
Descripción: 

Created on Thu Nov 16 15:42:36 2022

@author: jessi
"""

from Huella import Huella
import csv
with open('huellas.csv', 'w',newline="") as csvfile:
    writer = csv.writer(csvfile)
    for i in range(1,9):
        for j in range(1,3):
            print("Imagen: "+str(i)+"."+str(j))
            direccion = str(i)+"."+str(j)+".jpeg"
            #direccion = "ejem.jpg"
            huella = Huella(direccion)
            rasgos = huella.rasgos(huella.minu)
            writer.writerow([str(i)+"\t"+str(1)+":"+str(rasgos[0])+"\t"+str(2)+":"+str(rasgos[1])+"   "+str(3)+":"+str(rasgos[2])+"\t"+str(4)+":"+str(rasgos[3])+"\t"+str(5)+":"+str(rasgos[4])])