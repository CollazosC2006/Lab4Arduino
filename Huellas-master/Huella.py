# -*- coding: utf-8 -*-
"""
UNIVERSIDAD AUTÓNOMA DEL ESTADO DE MÉXICO
CU UAEM ZUMPANGO 
UA: Sistemas Expertos
TEMA:
Alumno: Jessica Vargas Sánchez 
Profesor: Mtra. Edith Cristina Herrena Luna
Descripción: 

Created on Thu Nov 8 21:40:32 2022

@author
"""

from Minucias import Minucia
import cv2
import numpy as np
from skimage.morphology import skeletonize, thin
import mahotas 
import mahotas as mh
import mahotas.demos 
from pylab import gray, imshow, show 

class Huella:
	def __init__(self,direccion):
		#nimg = input("Ingresa el nombre de la imagen: ")
		nimg = cv2.imread(direccion)
		#img = cv2.imread("./"+nimg)
        #Se obtiene el tamaño de la imagen
		self.X=230
		self.Y =300
		heigh = nimg.shape[0]
		width = nimg.shape[1]
		img = self.escalado(nimg, self.X, self.Y)
		#cv2.imshow("Escalado", img)


        #Se obtiene el nuevo tamaño de la imagen
		heigh = img.shape[0]
		width = img.shape[1]
		norm = self.Normalizacion(img, heigh, width)
		#cv2.imshow("Normalizacion", norm)
		
        
		fil = self.Filtrado(norm)
		#cv2.imshow("Filtrado", fil)
		
		bin = self.Binarizacion(fil, heigh, width, 50)
		#cv2.imshow("Binarizacion", bin)


		sk = self.Esqueletizacion(bin)
		sk = self.Binarizacion(sk, heigh, width, 50)
		#cv2.imshow("Esqueletizacion", sk)
		
		cbin = self.CamBinario(sk)
		#cv2.imshow("Inversion Binaria", cbin)

		self.minu = self.DeteccionM(cbin)
		pMin = self.PlotMinucias(cbin, self.minu)
		#cv2.imshow("Deteccion de Minucias", pMin)

		cv2.waitKey()


	def Normalizacion(self, img, heigh, width):
		normc = self.brillo(img, 70)
		normc = self.media(normc, heigh, width)
		return normc
    
	def Filtrado(self, img):
		#Filtro Gabor
		dk = 21
		g_kernel = cv2.getGaborKernel((dk, dk), 3, np.pi/2, 8, 8, 0, ktype=cv2.CV_32F)
		fil = cv2.filter2D(img, cv2.CV_8UC3, g_kernel)
		return fil
    
    
	def Binarizacion(self, img, heigh, width, umb):
		bin = self.umbral(img, heigh, width,umb)
		return bin
    
	def escalado(self, img, heigh, width):
		#print("\nEscalado de imagen")
		nheigh = heigh
		nwidth = width
		imagenEscalada = cv2.resize(img, (nheigh, nwidth), interpolation=cv2.INTER_AREA)
		return imagenEscalada
    
    
    
	def brillo(self, img, num):
		high = img.shape[0]
		width = img.shape[1]
		#print("\nBrillo de imagen")
		imagenB = np.zeros(((high), (width), 3), np.uint8)
		for k in range(3):
			for i in range(high):
				for j in range(width):
					if int(img[i, j, k] + num) > 255:
						imagenB[i, j, k] = 255
					else:
						imagenB[i, j, k] = img[i, j, k] + num
		return imagenB
    
	
	def umbral(self, img, high, width,umb):
		#print("\nBinarizacion de imagen")
		t, imagenUMB = cv2.threshold(img, 50, 255, cv2.THRESH_BINARY)
		return imagenUMB
    
    
    
	def media(self, img, high, width):
		media = np.zeros((int(high),int(width), 3), np.uint8)
		for q in range(3):
			for i in range(high):
				for j in range(width):
					m = []
					if i > 0 and i < (high-1):
						if j > 0 and j < (width-1):

                            #Obtenemos los valores de los vecinos de la vecindad
							m.append(img[i-1,j-1, q])
							m.append(img[i-1,j, q])
							m.append(img[i-1,j+1, q])
							m.append(img[i,j-1, q])
							m.append(img[i,j, q])
							m.append(img[i,j+1, q])
							m.append(img[i+1,j-1, q])
							m.append(img[i+1,j, q])
							m.append(img[i+1,j+1, q])
							val = 0
							for k in range(len(m)):
								val += m[k]
							#print(val)
							#Se asigna al pixel el promedio de la vecindad
							media[i,j,q] = val/9
		return media
    
    
	def Esqueletizacion(self, img):
		#print("\nEsqueletizacion")
		img = self.CamBinario(img)
		#img = img.max(2) 
		img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
		T_otsu = mahotas.otsu(img) 
		img = img > T_otsu 
		
		new_img = mahotas.thin(img) 	
		mh.imsave("esk.jpg",new_img)
		nimg = cv2.imread("esk.jpg")
		return nimg
    
	def CamBinario(self, img):
		heigh = img.shape[0]
		width = img.shape[1]
		kp = img.shape[2]
		#print("\nInversion Binarizacion de imagen")
		imagenBin = np.zeros((heigh, width, kp), np.uint8)
		for k in range(kp):
			for i in range(heigh):
				for j in range(width):
					pix = img[i, j, k]
					if pix == 255:
						imagenBin[i, j, k] = 0
					else:
						imagenBin[i, j, k] = 255
		return imagenBin
	def DeteccionM(self, img):
		#print("\nDetección de Minucias")
		heigh = img.shape[0]
		width = img.shape[1]
		p = 0
		min = []
		for i in range(heigh):
			for j in range(width):
				imgb = []
				if i > 0 and i < (heigh-1):
					if j > 0 and j < (width-1):
                        #Obtene el valor de los vecinos de cada pixel
						imgb.append(img[i-1, j-1, p])
						imgb.append(img[i-1, j, p])
						imgb.append(img[i-1, j+1, p])
						imgb.append(img[i, j+1, p])
						imgb.append(img[i+1, j+1, p])
						imgb.append(img[i+1, j, p])
						imgb.append(img[i+1, j-1, p])
						imgb.append(img[i, j-1, p])
						
						
						cn = 0
                        
						for b in range(len(imgb)):
							if(b < (len(imgb))-1):
								if imgb[b] == 255:
									imgb[b]=1
								elif imgb[b+1] == 255:
									imgb[b+1]=1
								elif imgb[b] == 0:
									imgb[b]=0
								elif imgb[b+1] == 0:
									imgb[b+1]=0

								cn += abs(imgb[b]-imgb[b+1])
								#print(type(cn))
						#2.5,3,
						cnb = (0.5*cn)
						cn = (cnb)/2.5
						minucia = Minucia()								
						if cn == 1.0:
							#print("La minucia es una terminacion")
							if(all(min) != [i, j, 1]):
								minucia.setMinucia(i, j, 1)															                                
								min.append(minucia)
							#print(imgb)
						
						cn=(cnb)/3.0
						if cn == 1.0:
							#print("La minucia es una terminacion")
							if(all(min) != [i, j, 1]):
								minucia.setMinucia(i, j, 1)															                                
								min.append(minucia)
							#print(imgb)
    
				
		return min
	def PlotMinucias(self, img, det):
		for i in range(len(det)):
			for j in range(2):
				if(j+1) < 2:
					x = det[i].getMinuta()[j]
					y = det[i].getMinuta()[j+1]
					cv2.circle(img,(y, x),2,(0,0,255),-1)
		cv2.imwrite("Minucias.jpg",img)
		return img
    
	def rasgos(self,minucias):
		t1 = 0
		t2 = 0
		t3 = 0
		t4 = 0
		totalTer =0
		for i in range(len(minucias)):
			minuta = minucias[i].getMinuta()
			x = minuta[0]
			y = minuta[1]
			tipo = minuta[2]
			#print(tipo)
			if x< self.X/2 and y<self.Y/2:
				t1 += 1
				totalTer += 1
                    
			if x> self.X/2 and y<self.Y/2:
				t2 += 1
				totalTer += 1

			if x< self.X/2 and y>self.Y/2:
				t3 += 1
				totalTer += 1                                
			if x> self.X/2 and y>self.Y/2:
				t4 += 1
				totalTer += 1
                    
		return [t1,t2,t3,t4,totalTer]