const paymentserverUrl = "http://localhost:8080"

const productDict = {
    lyft1: {
        ID: "lyft1",
        name: "Lyft Neon Static",
        price : 45,
        imageURL: "img/lyft-neon-static-white.png",
        type: 1
    },
    lyft2: {
        ID: "lyft2",
        name: "Lyft Ice Cool",
        price : 45,
        imageURL: "img/lyft-ice-cool.jpeg",
        type: 2
    },
    lyft3: {
        ID: "lyft3",
        name: "Lyft Tokyo Zing",
        price : 45,
        imageURL: "img/lyft-tokyo-zing.jpeg",
        type: 3
    }
}

var product

function buyProduct(ID) {
    location.href = "./buy.html?product=" + ID
}

function requestPayment(type) {
    window.location.href ='./thank_you_cash.html'
    displayLoading()
    fetch(paymentserverUrl + "/requestpayment?type=" + product.type, {
        method: "POST"
    })
    .then(res=>{
        if (res.status == 200) {
            hideLoading()     
        }
    })
}

function displayLoading() {
    document.getElementById('header').innerHTML = `<h3><font>Please pay ${product.price} with coins</font></h3>`
    document.getElementById('subtitle').innerHTML = "When you have paid, the product will be dispensed."
}

function hideLoading() {
    document.getElementById('header').innerHTML = `<h3><font>Thank you</font> for your purchase.</h3>`
    document.getElementById('subtitle').innerHTML = "The product will be dispensed."

}

function dispense() {
    if (product != null) {
    
        fetch(paymentserverUrl + "/dispense?type=" + product.type, {
            method: "POST"
        })
        .then(res=>{
            console.log(res)
            if (res.status == 200) {
                window.location.href ='./thank_you.html'
            }
        })
    } else {
        alert("Current selection is empty. Nothing was dispensed")
    }
}

function populate() {
    const params = new URLSearchParams(window.location.search)
    if (params.has('product')) {
        product = productDict[params.get('product')]
        console.log(product)
        document.getElementById('product-container').innerHTML = `	
        <div class="p-box-buy">
    		<img alt="1" src="${product.imageURL}" />
	    	<!--details--------->
		    <p>${product.name}</p>
		    <!--price--->
		    <a class="price" href="#">${product.price} DKK</a>
		    <a class="buy-cash-btn" onclick="requestPayment('${product.type}')" style="cursor: pointer;">Pay with coins</a>
            <div id="paypal-button-container"></div>
	    </div>
    `

    } else {
        product = null
        document.getElementById('product-container').innerHTML = "Go select a product in the store."
    }
}

function addProduct(ID) {
    let products = []
    if (localStorage.getItem('products')){
        products = JSON.parse(localStorage.getItem('products'))
    }
    products.push(productDict[ID])
    localStorage.setItem('products', JSON.stringify(products))
    updateCount()
}

function removeProduct(ID) {
    let products = JSON.parse(localStorage.getItem('products'))
    let updatedProducts = products.filter(product => product.ID !== ID)
    localStorage.setItem('products', JSON.stringify(updatedProducts))
}
function countProducts() {
    let products = JSON.parse(localStorage.getItem("products"))
    console.log(Object.keys(products))
    let count = Object.keys(products).length
    console.log("Item added: ", count)
    return count
}

function updateCount() {
    let element = document.getElementById("cartCount")
    if (element !== null){
        element.innerHTML = countProducts()
    }
}


