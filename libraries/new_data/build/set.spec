#include bool.spec

sort Set(S) <"set_">;

map @set <"set_comprehension"> : (S -> Bool) <"arg"> -> Set(S);
    {} <"emptyset"> : Set(S);
    in <"setin"> : S <"left"> # Set(S) <"right"> -> Bool;
    + <"setunion_"> : Set(S) <"left"> # Set(S) <"right"> -> Set(S);
    - <"setdifference"> : Set(S) <"left"> # Set(S) <"right"> -> Set(S);
    * <"setintersection"> : Set(S) <"left"> # Set(S) <"right"> -> Set(S);
    ! <"setcomplement"> : Set(S) <"arg"> -> Set(S);

var d : S;
    s : Set(S);
    t : Set(S);
    f : S->Bool;
    g : S->Bool;
eqn ==(@set(f),@set(g)) = ==(f,g);
    {} = @set(lambda(x:S, false));
    in(d,@set(f)) = f(d);
    <=(@set(f),@set(g)) = forall(x:S, =>(f(x),g(x)));
    <(s,t) = &&(<=(s,t), !=(s,t));
    +(@set(f),@set(g)) = @set(lambda(x:S, ||(f(x), g(x))));
    -(s,t) = *(s, !(t));
    *(@set(f),@set(g)) = @set(lambda(x:S, &&(f(x), g(x))));
    !(@set(f)) = @set(lambda(x:S, !(f(x))));

