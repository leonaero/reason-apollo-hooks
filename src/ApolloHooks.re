module Mutation = ApolloHooksMutation;
module Query = ApolloHooksQuery;
module LazyQuery = ApolloHooksLazyQuery;
module Provider = ApolloHooksProvider;
module Subscription = ApolloHooksSubscription;

let useQuery = Query.useQuery;
let useLazyQuery = LazyQuery.useLazyQuery;
let useMutation = Mutation.useMutation;
let useSubscription = Subscription.useSubscription;

let toQueryObj = (result): ApolloClient.queryObj => {
  "query": ApolloClient.gql(. result##query),
  "variables": result##variables,
};

let toReadQueryOptions = result => {
  "query": ApolloClient.gql(. result##query),
  "variables": Js.Nullable.fromOption(Some(result##variables)),
};
