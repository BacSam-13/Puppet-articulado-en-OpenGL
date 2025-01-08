# Puppet-articulado-en-OpenGL
Puppet completamente artículado con jerarquía, utilizando OpenGL.


El proyecto aquí presente consiste en la creación de un Puppet artículado utilizando las herramientas de OpenGL.
El puppet en cuestión esta completamente artículado, respetando la jerarquía del cuerpo, y se puede visualizar desde cualquier ángulo.


--Ejecución del Código
Propiedades del proyecto: Al momento de generar un proyecto en C++ con Visual Studio, e icluyendo los documentos como se encuentran adjuntos, se debe incluir
lo siguiente dentro de las propiedades del proyecto:

	C/C++ \ General \ Directorios de inclusion adicionales: $(SolutionDir)Dependencies\GLM;$(SolutionDir)Dependencies\GLFW\include;$(SolutionDir)Dependencies\GLEW\include
 
	Vinculador \ General \ Directorios de Bibliotecas Adicionales: $(SolutionDir)Dependencies\GLFW\lib-vc2015;$(SolutionDir)Dependencies\GLEW\lib\Release\Win32
 
	Vinculador \ Entrada \ Dependencias Adicionales: glew32s.lib;glfw3.lib;opengl32.lib;User32.lib;Gdi32.lib;Shell32.lib
 
Al realizar esto, el proyecto en sí debería funcionar sin problema alguno, simplemente al compilarlo y ejecutarlo.

--Problemas del Código
Durante el desarrollo de este codigo, se presentaron numerosos problemas, los cuales eventualmente se pudieron resolver, sobrellevar, o adaptar. Los principales problemas
son los siguientes:

	-Uso de glm: Tras descargar e incluir dentro de las dependencias del proyecto la ultima version de glm, el codigo fallaba sin aparente solucion. Se requirio de instalar
	distintas versiones para que finalmente una permitiera su correcta compilacion.
 
	-Depth: Durante la implementacion de los shaders, se intento emplear modelos de luz, pero se opto por un coloreado de los objetos que permitiera el sistinguir su orientacion.
	Sin embargo, los cubos dibujados terminadan con caras trasnsparentes, y otras sobrepuestas. Se penso que se debia al uso de las normas en el fragment shader, pero unicamente
	se debia de implementar la profundidad para nuestra ventana
 
	-ImGui: Esta libreria presento ciertos inconventienes. Debido a que con esta libreria podemos generar ventanas para cambiar en tiempo real los parametros del puppet,
	se termino por volver demasiado grande debido a la abundancia de los mismos. Se intento emplear numerosas estrategias para reducir el tamaño de la ventana resultante,
	pero debido al nulo progreso, se opto por dejar la ventana en su estado actual. Debido a que no afecta en nada el buen fucionamiento del codigo.


--Referencias consultadas:

-Foundations of 3D Computer Graphics,  Steven J. Gortler

-The Cherno/OpenGL
