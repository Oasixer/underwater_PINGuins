<script>
    import { onMount } from 'svelte';

    async function get() {
        // const response = await fetch('http://192.168.1.70:8000/');
        const response = await fetch('http://localhost:8000/');
        const data = await response.json();

        return {
            body: {
            data,
            },
        };
    }
    // call get on mount
    onMount(async () => {
        console.log(get());
    });

</script>
<div>
    <p>blah</p>
</div>