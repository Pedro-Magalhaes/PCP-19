package main
import (
	"fmt"
	"math"
	"time"
    "os"
	"strconv"
    "sync"
    "sync/atomic"
    "unsafe"
)

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

    
    var incremento = math.Abs((fim - inicio) / float64(num_threads) );
        
    
    var wg sync.WaitGroup;
    var area = 0.0;
    // Start time
    start := time.Now();
    
    
    for index := 0; index < num_threads; index++ {
        wg.Add(1);
        var a = incremento * float64(index) + inicio;
        var b = a + float64(incremento);
        if index == num_threads-1 {
            b = fim;
        }
        var t = Tarefa{
            a: a,
            b: b,
            area: 0,
            calculated: false,
        }
        go func() {
            var area_local = calcula_area1(t,tol,fx);
            AtomicAddFloat64(&area,area_local);
            wg.Done()
        }()
    }
      
    
    wg.Wait()

    // End
    elapsed := time.Since(start)
    fmt.Println("area ", area);
    fmt.Println("tempo ", elapsed.Seconds())

}

func AtomicAddFloat64(val *float64, delta float64) (new float64) {
    for {
        old := *val
        new = old + delta
        if atomic.CompareAndSwapUint64(
            (*uint64)(unsafe.Pointer(val)),
            math.Float64bits(old),
            math.Float64bits(new),
        ) {
            break
        }
    }
    return
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

func fx(x float64) float64 {
    return math.Exp(x) * math.Cos(x);
}


func calcula_area1( t Tarefa, tol float64, f func(float64) float64) float64{
    // println!("Thread {:?} trabalhando",thread::current().id());
    var h = math.Abs(t.b-t.a);
    var meio = (t.a+t.b)/2.0;
    var hmeio = h/2.0;

    var fa = f(t.a);
    var fb = f(t.b);
    var fm = f(meio);
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
