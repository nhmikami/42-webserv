const fast = document.getElementById("fast");
const silent = document.getElementById("silent");
const deadly = document.getElementById("deadly");

fast.addEventListener('click', () => changeRoute("./fast/"))
silent.addEventListener('click', () => changeRoute("./silent/"))
deadly.addEventListener('click', () => changeRoute("./deadly/"))

function changeRoute(route){
    window.location.href = route;
}