# -*- coding: utf-8 -*-
"""
UNIVERSIDAD AUTÓNOMA DEL ESTADO DE MÉXICO
CU UAEM ZUMPANGO 
UA: Sistemas Expertos
TEMA:
Alumno: Jessica Vargas Sánchez 
Profesor: Mtra. Edith Cristina Herrena Luna
Descripción: 

Created on Thu Nov 16 15:06:35 2022

@author: jessi
"""

import sys
import PySimpleGUI as sg
from Huella import Huella
import numpy as np
import sklearn
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.neighbors import KNeighborsClassifier
from sklearn.datasets import load_svmlight_file

class Interfaz:
    def __init__(self):

        archivo = "huellas.csv"
        #archivo = "Iris_DB_svm.txt"
        x_train, y_train = load_svmlight_file(archivo)
        x_entrenar, x_test, y_entrenar, y_test = train_test_split(x_train,y_train)
        knn = KNeighborsClassifier( n_neighbors=10 )
        self.knn =knn.fit(x_entrenar, y_entrenar)
        aprend = knn.score(x_test, y_test)
        
        sg.ChangeLookAndFeel('Material1')
        layout = [[sg.Text('Inicio de Sesion', size=(40, 1), justification='center')],
                  [sg.Text(text='Inicio de Sesion', justification='center')],
                  [sg.Text(text='Huella')],
                  [sg.Input(), sg.FileBrowse()],
                  [sg.Button('Iniciar Sesion', key='validar'), sg.Button('Cancelar', key = 'cancelar')]
                  ]
        self.window = sg.Window('Inicio de Sesion', location=(800, 400))
        self.window.Layout(layout).Finalize()
        while True:
            event, values = self.window.Read()
            if event == 'Exit' or event is None:
                sys.exit()
                break
            if event == 'validar':
                if(values[0]==""):
                    sg.Popup('Ingrese una dirección')
                else:
                    clase = self.validar(values[0])
                    print(clase)
                    if(clase==1):
                        sg.Popup('Bienvenido: Amaury')
                    elif(clase==2):
                        sg.Popup('Bienvenido: Bryan')
                    elif(clase==3):
                        sg.Popup('Bienvenido: Armando')
                        
                    elif(clase==4):
                        sg.Popup('Bienvenido: Jessica')
                    elif(clase==5):
                        sg.Popup('Bienvenido: Erick')
                       
                    elif(clase==6):
                        sg.Popup('Bienvenido: Ana')
                    elif(clase==7):
                        sg.Popup('Bienvenido: Ceci')
                    elif(clase==8):
                        sg.Popup('Bienvenido: Cesar')
                    values[0]('')
                        
                
            if event == 'cancelar':
                self.window.close()
                sys.exit()
            
    def validar(self, direccion):
        huella = Huella(direccion)
        clasif = self.knn.predict([huella.rasgos(huella.minu)])
        clasif = np.asarray(clasif, dtype = int)
        c= int(0 if clasif[0] is None else clasif[0])
        return c
        
        
        
    
inter = Interfaz()