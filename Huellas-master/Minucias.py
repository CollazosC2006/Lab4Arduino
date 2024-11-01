# -*- coding: utf-8 -*-
"""
UNIVERSIDAD AUTÓNOMA DEL ESTADO DE MÉXICO
CU UAEM ZUMPANGO 
UA: Sistemas Expertos
TEMA:
Alumno: Jessica Vargas Sánchez 
Profesor: Mtra. Edith Cristina Herrena Luna
Descripción: 

Created on Thu Nov 10 22:02:32 2022

@author: jessi
"""
class Minucia:
    def __init__(self):
        pass
    def setMinucia(self, posicionX, posicionY,tipo):
        self.posicionX = posicionX
        self.posicionY = posicionY
        self.tipo = tipo
        
        
    def getMinuta(self):
        return [self.posicionX, self.posicionY,self.tipo]
    