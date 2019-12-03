package main

import (
	"fmt"
	"math"
)

type Vertex struct {
	X int
	Y int
}

type Tarefa struct {
    a float64
    b float64
    area float64
    calculated bool
}

func main() {
    var t = Tarefa{
        a: 0,
        b: 1,
        area: 0,
        calculated: false,
    }

    var area = calcula_area1(t,0.001,fx2);
    fmt.Println("area  ", area);
}



func x2(x float64) float64 {
    return x * x;
}

// Funcoes com carga de processamento relevante
func fx1(x float64) float64 {
    var res float64 = 0.0;
    for i:=0; i < 10000; i++ {
        res += math.Exp(3.0*x) * math.Sin(2.0*x);
    }
    return res;
}

func fx2(x float64) float64 {
    var res float64 = 0.0;
    for i:=0; i < 100000; i++ {
        res += math.Exp(x);
    }
    return res;
}

func calcula_area1( t Tarefa, tol float64, f func(float64) float64) float64{
    // println!("Thread {:?} trabalhando",thread::current().id());
    var h = math.Abs(t.b-t.a);
    var meio = (t.a+t.b)/2.0;
    var hmeio = h/2.0;
    
    var fa = math.Abs(f(t.a));
    var fb = math.Abs(f(t.b));
    var fm = math.Abs(f(meio));
    var area_trapezio_maior =  ((fa + fb) * h) / 2.0;
    var area_trapezios =  (((fa + fm) * hmeio) / 2.0 )+ (((fb + fm) * hmeio) / 2.0);
    
    if math.Abs(area_trapezio_maior - area_trapezios) > tol {
        // println!("rec");
        var t1 = Tarefa{ a: t.a, b: meio, area: 0.0, calculated: false };
        var t2 = Tarefa { a: meio, b: t.b, area: 0.0, calculated: false };
        area_trapezios = calcula_area1(t1,tol,f) + calcula_area1(t2,tol,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }
    return area_trapezios;
}