package main
import (
	"fmt"
	"math"
	"time"
  "os"
	"strconv"
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
		var num_args = len(os.Args[0:])
		if num_args < 4 {
				fmt.Printf("Erro, os parametros necessários não foram informados.")
				fmt.Printf("\nO Programa recebe os parametros nesta ordem:")
				fmt.Printf("\n\t<numero de threads>\n\t<valor de 'a'>\n\t<valor de 'b'>")
				fmt.Printf("\n\t<tolerancia>")
				fmt.Printf("\nNumero de parametros recebidos: %v\n\n", num_args);
				os.Exit(2)
		}

		var num_threads, err1 = strconv.Atoi(os.Args[1]); if err1 != nil {
			fmt.Println(err1)
			os.Exit(2)
		}
		var inicio, err2 = strconv.ParseFloat(os.Args[2], 64); if err2 != nil {
			fmt.Println(err2)
			os.Exit(2)
		}
		var fim, err3 = strconv.ParseFloat(os.Args[3], 64); if err3 != nil {
			fmt.Println(err3)
			os.Exit(2)
		}
		var tol, err4 = strconv.ParseFloat(os.Args[4], 64); if err4 != nil {
			fmt.Println(err4)
			os.Exit(2)
		}

		fmt.Printf("type: %T %v [num_threads]\n", num_threads, num_threads)
		fmt.Printf("type: %T %v [inicio]\n", inicio, inicio)
		fmt.Printf("type: %T %v [fim]\n", fim, fim)
		fmt.Printf("type: %T %v [tol]\n\n", tol, tol)

    var t = Tarefa{
        a: inicio,
        b: fim,
        area: 0,
        calculated: false,
    }

		// Start
		start := time.Now()

		var area = calcula_area1(t,tol,fx2);

		// End
		elapsed := time.Since(start)
		fmt.Println("area ", area);
    fmt.Printf("tempo %secs\n", elapsed)

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
