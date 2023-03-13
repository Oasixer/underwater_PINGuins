<script>
    import { onMount } from 'svelte';

    async function get() {
        const response = await fetch('/test');
        const data = await response.json();

        console.log(`${data}`);
        console.log(`${data.counter}`);
        return data.counter;
    }

  let counter = 0;

  onMount(() => {
    // Schedule the `get()` function to run every 500ms (half second)
    const intervalId = setInterval(async () => {
      counter = await get();
    }, 500);

    // Clean up the interval when the component is unmounted
    return () => clearInterval(intervalId);
  });
    // call get on mount
    // onMount(async () => {
    //     counter = get();
    // });

</script>
<div>
    <p>blah{counter}</p>
</div>